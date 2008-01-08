#include <string.h>
#include "endian.h"
#include "arp.h"
#include "nettypes.h"
extern dev_t *dev;
typedef struct {
	u8_t dest[6];
	u8_t src[6];
	u16_t type;
} __attribute__((__packed__)) ethernet_hdr;
int CreateEthernetHeader(u8_t *target, u16_t type, void *buf) {
	ethernet_hdr *hdr = buf;
	memcpy(hdr->dest, target, 6);
	memcpy(hdr->src, dev->dev_addr, 6);
	hdr->type = htons(type);
	return sizeof(ethernet_hdr);
}

int EthernetSent(void *buf, int size) {
	memcpy(dev->packet, buf, size);
	dev->packetlen = size;
	(dev->device_transmit)(dev);
	return 0;
}

void EthernetMyIP(void *buf) {
	memcpy(buf, &dev->ip, 4);
}

void EthernetMyMAC(void *buf) {
	memcpy(buf, dev->dev_addr, 6);
}

u16_t EthernetType(void *buf) {
	ethernet_hdr *hdr = buf;
	u16_t type = ntohs(hdr->type);
	if(type > 0x05DC)	// это Ethernet II
		return type;
	else	// это IEEE 802.3, пшел вон
		return 0;
}

int EthernetGetTarget(u32_t ip, u8_t *mac) {
	if((ip & dev->mask) == (dev->ip & dev->mask)) {
		printf("Sending to same segment\n");
		if(arp_get_mac(ip, mac) < 0)
			return -1;
		return 0;
	} else{
		printf("Sending via gateway\n");
		if(arp_get_mac(dev->gateway, mac) < 0)
			return -1;
		return 0;
	}	
}
