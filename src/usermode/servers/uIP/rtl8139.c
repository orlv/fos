#include <sys/pci.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/io.h>
#include <types.h>
#include <sched.h>
#include <fos/message.h>
#include <fos/fos.h>
#include <sys/mman.h>
#include <fos/page.h>
#include <string.h>
#include "uip.h"
#undef CARD_DEBUG

#define ETH_ZLEN	60
#define ETH_FRAME_LEN	1514
#define TX_FIFO_THRESH	256
#define RX_FIFO_THRESH	4
#define RX_BUF_LEN_IDX	0
#define TX_DMA_BURST	4
#define RX_DMA_BURST	4
#define RX_BUF_LEN	(8192 << RX_BUF_LEN_IDX)
#define TARGET		0xB900

#define CONFIG1		0x52
#define CFG9346		0x50
#define MII_BMCR 	0x62
#define CHIPCMD		0x37
#define MAC0		0
#define RXCONFIG	0x44
#define TXCONFIG	0x40
#define RXBUF		0x30
#define RXMISSED	0x4C
#define RXCONFIG	0x44
#define INTRMASK	0x3C
#define MAR0		8
#define INTRSTATUS	0x3E
#define RXBUFPTR	0x38
#define TXADDR0		0x20
#define TXSTATUS0	0x10

#define MEDIASTATUS	0x58
#define MSRSPEED10	0x08
#define BMCR_DUPLEX	0x100
#define MSRLINKFAIL	0x04

#define CMDRESET	0x10
#define CMD_RX_ENB	0x08
#define CMD_TX_ENB	0x04
#define RXBUFEMPTY	0x01

#define TX_BUF_SIZE	ETH_FRAME_LEN
#define NUM_TX_DESC	4

#define ACCEPT_BROADCAST	0x08
#define ACCEPT_MY_PHYS		0x02
#define ACCEPT_ALL_PHYS		0x01

#define PCIERR		0x8000
#define PCSTIMEOUT	0x4000
#define RXFIFOOVER	0x40
#define RXUNDERRUN	0x20
#define RXOVERFLOW	0x10
#define TXERR		0x08
#define TXOK		0x04
#define RXERR		0x02
#define RXOK		0x01

#define RXBADSYMBOL	0x20
#define RXRUNT		0x10
#define RXTOOLONG	0x08
#define RXCRCERR	0x04
#define RXBADALIGN	0x02

#define RX_CONFIG ((RX_BUF_LEN_IDX << 11) | (RX_FIFO_THRESH << 13) | (RX_DMA_BURST << 8))
typedef struct {
	int io;
	int irq;
	int rx_buf_len;
	void *rx_ring;
	int promisc;
	int cur_rx, cur_tx;
	char *tx_buffer;
	int packets;
} rtl8139_data;

rtl8139_data data;
u8_t rtl8139_mac[8];

static int read_eeprom(long ioaddr, int location, int addr_len);
static void rtl8139_reset(int ioaddr);
int rtl8139_poll();
void rtl8139_transmit();

int rtl8139_init(pci_addr_t *addr) {
	int i;
	addr->enable = 1;


	data.io = pci_inl(addr, 0x10) & 0xFFFFFF00;
	data.irq = pci_inb(addr, 0x3C);

	data.promisc = 0; // важно!

	int io = data.io;
	printf("I/O at %X, ", data.io);
	printf("IRQ %u, ", data.irq); 
	outb(0x00, io + CONFIG1);
	
	printf("ring %u bytes, ", RX_BUF_LEN + 16 + (TX_BUF_SIZE * NUM_TX_DESC));
	data.rx_ring = kmmap(0, RX_BUF_LEN + 16 + (TX_BUF_SIZE * NUM_TX_DESC), 0, 0xB9000);
	data.tx_buffer = kmmap(0, ETH_FRAME_LEN, 0, 0);
	data.rx_buf_len = RX_BUF_LEN;
	int addr_len = read_eeprom(io, 0, 8) == 0x8129 ? 8 : 6;
	for(i = 0; i < 3; i++) 
		((u16_t *)(rtl8139_mac))[i] = read_eeprom(io, i + 7, addr_len);
	
	printf("MAC address: ");
	for(i = 0; i < 5; i++) 
		printf("%2.2x:", rtl8139_mac[i]);
	printf("%2.2x, ", rtl8139_mac[i]);

	int speed10 = inb(io + MEDIASTATUS) & MSRSPEED10;
	int fullduplex = inw(io + MII_BMCR) & BMCR_DUPLEX;
	
	printf("%sMbps %s-duplex\n", speed10 ? "10" : "100", fullduplex ? "full" : "half");
	rtl8139_reset(io);
	if(inb(io + MEDIASTATUS) & MSRLINKFAIL) {
		printf("Link failure, aborting!\n");

		kmunmap((off_t) data.rx_ring, data.rx_buf_len + 16 + (TX_BUF_SIZE * NUM_TX_DESC)); 

		return -1;
	}else {
		return 0;
	}


}
static void rtl8139_reset(int ioaddr) {
	data.cur_rx = 0;
	data.cur_tx = 0;
	outb(CMDRESET, ioaddr + CHIPCMD);
	int start = uptime();
	while(inb(ioaddr + CHIPCMD) & CMDRESET && (uptime() - start) < 10) sched_yield();

	outb(0xC0, ioaddr + CFG9346);
	outl(*(u32_t *)(rtl8139_mac + 0), ioaddr + MAC0 + 0);
	outl(*(u32_t *)(rtl8139_mac + 4), ioaddr + MAC0 + 4);

	outb(CMD_RX_ENB | CMD_TX_ENB, ioaddr + CHIPCMD);

	outl((RX_FIFO_THRESH << 13) | (RX_BUF_LEN_IDX << 11) | (RX_DMA_BURST << 8), ioaddr + RXCONFIG);

	outl((TX_DMA_BURST << 8) | 0x03000000, ioaddr + TXCONFIG);	

	outl(getpagephysaddr((off_t) data.rx_ring), ioaddr + RXBUF);

	outb(CMD_RX_ENB | CMD_TX_ENB, ioaddr + CHIPCMD);

	outl(RX_CONFIG, ioaddr + RXCONFIG);

	outl(0, ioaddr + RXMISSED);


	outl(RX_CONFIG | ACCEPT_BROADCAST | ACCEPT_MY_PHYS , ioaddr + RXCONFIG);
	outl(0xffffffff, ioaddr + MAR0 + 0);
	outl(0xffffffff, ioaddr + MAR0 + 4);
//	!! outb(ACCEPT_BROADCAST | ACCEPT_MY_PHYS | data.promisc ? ACCEPT_ALL_PHYS : 0, ioaddr + RXCONFIG);

	

//	outw(PCIERR | PCSTIMEOUT | RXUNDERRUN | RXOVERFLOW | RXFIFOOVER |
//		TXERR | TXOK | RXERR | RXOK, ioaddr + INTRMASK);
	outw(0, ioaddr + INTRMASK);
}
#define EE_SHIFT_CLK	0x04	/* EEPROM shift clock. */
#define EE_CS		0x08	/* EEPROM chip select. */
#define EE_DATA_WRITE	0x02	/* EEPROM chip data in. */
#define EE_WRITE_0	0x00
#define EE_WRITE_1	0x02
#define EE_DATA_READ	0x01	/* EEPROM chip data out. */
#define EE_ENB		(0x80 | EE_CS)
#define EE_WRITE_CMD	(5)
#define EE_READ_CMD	(6)
#define EE_ERASE_CMD	(7)
static int read_eeprom(long ioaddr, int location, int addr_len) {
	long ee_addr = ioaddr + CFG9346;
	int read_cmd = location | (EE_READ_CMD << addr_len);
	int retval = 0;
	outb(EE_ENB & ~EE_CS, ee_addr);
	outb(EE_ENB, ee_addr);

	for(int i = 4 + addr_len; i >= 0; i--) {
		int dataval = (read_cmd & (1 << i)) ? EE_DATA_WRITE : 0;
		outb(EE_ENB | dataval, ee_addr);
		inb(ee_addr);
		outb(EE_ENB | dataval | EE_SHIFT_CLK, ee_addr);
		inb(ee_addr);
	}
	outb(EE_ENB, ee_addr);
	inb(ee_addr);
	
	for(int i = 16; i > 0; i--) {
		outb(EE_ENB | EE_SHIFT_CLK, ee_addr);
		inb(ee_addr);
		retval = (retval << 1) | ((inb(ee_addr) & EE_DATA_READ) ? 1 : 0);
		outb(EE_ENB, ee_addr);
		inb(ee_addr);
	}
	outb(~EE_CS, ee_addr);
	return retval;
}

int rtl8139_poll() {

	int ioaddr = data.io;
	while(inb(ioaddr + CHIPCMD) & RXBUFEMPTY) sched_yield();

	//printf("$$$ %u packets get\n", ++data.packets);
	int status = inw(ioaddr + INTRSTATUS);
	outw(status & ~ (RXFIFOOVER | RXOVERFLOW | RXOK), ioaddr + INTRSTATUS);
	int ring_offs = data.cur_rx % RX_BUF_LEN;
	int rx_status = *(unsigned int*)(data.rx_ring + ring_offs);
	int rx_size = rx_status >> 16;
	rx_status &= 0xFFFF;

	if((rx_status & (RXBADSYMBOL | RXRUNT | RXTOOLONG | RXCRCERR | RXBADALIGN)) || (rx_size < ETH_ZLEN) || (rx_size > ETH_FRAME_LEN + 4)) {
#ifdef CARD_DEBUG
		printf("WARNING: rx error %hX, size %u bytes\n", rx_status, rx_size);
#endif
		rtl8139_reset(ioaddr);
		return 0;
	}

	int packetlen = rx_size - 4;
//	printf("Packet size - %u bytes\n", dev->packetlen);
	if(ring_offs + 4 + rx_size - 4 > RX_BUF_LEN) {
		int semi_count = RX_BUF_LEN - ring_offs - 4;
		memcpy(uip_buf, data.rx_ring + ring_offs + 4, semi_count);
		memcpy(uip_buf + semi_count, data.rx_ring, rx_size - 4 - semi_count);
#ifdef CARD_DEBUG
		printf("rx packet %u+%u bytes\n", semi_count, rx_size - 4 - semi_count);
#endif
	} else {
		memcpy(uip_buf, data.rx_ring + ring_offs + 4, packetlen);
#ifdef CARD_DEBUG
		printf("rx packet %u bytes\n", rx_size - 4);
#endif
	}
#ifdef CARD_DEBUG
	printf(" start %X offset %x  at %X rxstatus %hX\n", data.rx_ring, ring_offs + 4,
		(unsigned long)(data.rx_ring + ring_offs + 4),
		rx_status);
#endif
	data.cur_rx = (data.cur_rx + rx_size + 4 + 3) & ~3;

	outw(data.cur_rx - 16, ioaddr + RXBUFPTR);
	outw(status & (RXFIFOOVER | RXOVERFLOW | RXOK), ioaddr + INTRSTATUS);
	return packetlen;
}

void rtl8139_transmit() {

	int ioaddr = data.io;
	int packetlen = uip_len;
	memcpy(data.tx_buffer, uip_buf, packetlen);
	while(packetlen < ETH_ZLEN) {
		data.tx_buffer[packetlen++] = 0;
	}

	outl(getpagephysaddr((off_t) data.tx_buffer), ioaddr + TXADDR0 + data.cur_tx * 4);
	outl(((TX_FIFO_THRESH << 11) & 0x003f0000) | packetlen, ioaddr + TXSTATUS0 + data.cur_tx * 4);
	
	u32_t to = uptime() + 1000;
	u32_t status;
	do {
		status = inw(ioaddr + INTRSTATUS);
		outw(status & (TXOK | TXERR | PCIERR), ioaddr + INTRSTATUS);
		if(status & (TXOK | TXERR | PCIERR)) break;
	} while(uptime() < to);

	u32_t txstatus = inl(ioaddr + TXSTATUS0 + data.cur_tx * 4);
	if(status & TXOK) {
		data.cur_tx = (data.cur_tx + 1) % NUM_TX_DESC;
#ifdef CARD_DEBUG
		printf("tx done, status %hX txstatus %X\n",
			status, txstatus);
#endif
	} else {
		printf("tx timeout/error (%d ticks), status %hX txstatus %X\n",
			uptime() - to, status, txstatus);
		rtl8139_reset(ioaddr);
	}
}
