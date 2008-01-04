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
#include "nettypes.h"

#define TX_FIFO_THRESH	256
#define RX_FIFO_THRESH	4
#define RX_BUF_LEN_IDX	2
#define TX_DMA_BURST	4
#define RX_DMA_BURST	4

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

#define MEDIASTATUS	0x58
#define MSRSPEED10	0x08
#define BMCR_DUPLEX	0x100
#define MSRLINKFAIL	0x04

#define CMDRESET	0x10
#define CMD_RX_ENB	0x08
#define CMD_TX_ENB	0x04

#define TX_BUF_SIZE	1536
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
typedef struct {
	int io;
	int irq;
	int rx_buf_len;
	void *rx_ring;
	int promisc;
} rtl8139_data;

static int read_eeprom(long ioaddr, int location, int addr_len);
static void rtl8139_reset(int ioaddr, dev_t *dev);

volatile dev_t *forint;
THREAD(rtl8139_interrupt) {
	struct message msg;
	struct dev *dev = (dev_t *)forint;
	rtl8139_data *data = dev->custom;
	printf("RTL8139 interrupts handler (for irq %u)\n", data->irq);
	if(interrupt_attach(data->irq) != RES_SUCCESS) {
		printf("can't attach!\n");
		return 1;
	}
	unmask_interrupt(data->irq);
	while (1) {
		msg.tid = _MSG_SENDER_SIGNAL;
		msg.recv_size = 0;
		receive(&msg);
		if(msg.arg[0] == data->irq) {
			printf("int\n");
			unmask_interrupt(data->irq);
		} else
			printf("not my interrupt\n");
		reply(&msg);
	}
}

int rtl8139_init(net_callbacks_t *callbacks, struct dev *dev) {
	int i;
	dev->addr->enable = 1;
	rtl8139_data *mydata = malloc(sizeof(rtl8139_data));
	dev->custom = mydata;
	mydata->io = pci_inl(dev->addr, 0x10) & 0xFFFFFF00;
	mydata->irq = pci_inb(dev->addr, 0x3C);

	mydata->promisc = 1; // важно!

	int io = mydata->io;
	printf("I/O at %X, ", mydata->io);
	printf("IRQ %u, ", mydata->irq); 
	outb(0x00, io + CONFIG1);
	
	mydata->rx_buf_len = 8192 << RX_BUF_LEN_IDX;
	//FIXME: выравнивать на страницу, получать ФИЗИЧЕСКИЙ АДРЕС
	printf("ring %u bytes, ", mydata->rx_buf_len + 16 + (TX_BUF_SIZE * NUM_TX_DESC));
	mydata->rx_ring = kmmap(0, mydata->rx_buf_len + 16 + (TX_BUF_SIZE * NUM_TX_DESC), 0, 0);


	int addr_len = read_eeprom(io, 0, 8) == 0x8129 ? 8 : 6;
	for(i = 0; i < 3; i++) 
		((u16_t *)(dev->dev_addr))[i] = read_eeprom(io, i + 7, addr_len);
	
	printf("MAC address: ");
	for(i = 0; i < 5; i++) 
		printf("%2.2x:", dev->dev_addr[i]);
	printf("%2.2x, ", dev->dev_addr[i]);

	int speed10 = inb(io + MEDIASTATUS) & MSRSPEED10;
	int fullduplex = inw(io + MII_BMCR) & BMCR_DUPLEX;
	
	printf("%sMbps %s-duplex\n", speed10 ? "10" : "100", fullduplex ? "full" : "half");
	rtl8139_reset(io, dev);
	if(inb(io + MEDIASTATUS) & MSRLINKFAIL) {
		printf("Link failure, aboring!\n");
		mask_interrupt(mydata->irq);
		kmunmap((off_t) mydata->rx_ring, mydata->rx_buf_len + 16 + (TX_BUF_SIZE * NUM_TX_DESC)); // FIXME: см выше
		free(mydata);
		return -1;
	}else {
		forint = dev;
		thread_create((off_t ) & rtl8139_interrupt);
		return 0;
	}


}
static void rtl8139_reset(int ioaddr, dev_t *dev) {
	rtl8139_data *data = dev->custom;
	outb(CMDRESET, ioaddr + CHIPCMD);
	int start = uptime();
	while(inb(ioaddr + CHIPCMD) & CMDRESET && (uptime() - start) < 10) sched_yield();

	outb(0xC0, ioaddr + CFG9346);
	outl(*(u32_t *)(dev->dev_addr + 0), ioaddr + MAC0 + 0);
	outl(*(u32_t *)(dev->dev_addr + 4), ioaddr + MAC0 + 4);

	outb(CMD_RX_ENB | CMD_TX_ENB, ioaddr + CHIPCMD);
	outl((RX_FIFO_THRESH << 13) | (RX_BUF_LEN_IDX << 11) | (RX_DMA_BURST << 8), ioaddr + RXCONFIG);
	outl((TX_DMA_BURST << 8) | 0x03000000, ioaddr + TXCONFIG);	

	printf("%x resolving to ", data->rx_ring);
	printf("%x\n", getpagephysaddr((off_t) data->rx_ring));
	printf("!");
	 outl(getpagephysaddr((off_t) data->rx_ring), ioaddr + RXBUF);
	printf("!");
	outl(0, ioaddr + RXMISSED);
	outb(ACCEPT_BROADCAST | ACCEPT_MY_PHYS | ((rtl8139_data *)(dev->custom))->promisc ? ACCEPT_ALL_PHYS : 0, ioaddr + RXCONFIG);
	outb(CMD_RX_ENB | CMD_TX_ENB, ioaddr + CHIPCMD);

	outw(PCIERR | PCSTIMEOUT | RXUNDERRUN | RXOVERFLOW | RXFIFOOVER |
		TXERR | TXOK | RXERR | RXOK, ioaddr + INTRMASK);
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
