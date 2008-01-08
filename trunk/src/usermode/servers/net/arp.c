#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fos/fos.h>
#include <sched.h>
#include "ethernet.h"
#include "endian.h"
#include "stackconfig.h"
static u8_t arp_target[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

struct {
	u32_t ip;
	u8_t mac[6];
	u8_t present;
	u32_t created;
} arp_cache[ARP_CACHE_SIZE];
typedef struct {
	u16_t	hw_addr_type;
	u16_t	pr_addr_type;
	u8_t	hw_addr_size;
	u8_t	pr_addr_size;
	u16_t	operation;
	u8_t	hw_sender[6];
	u32_t	ip_sender;
	u8_t	hw_target[6];
	u32_t	ip_target;
} __attribute__((__packed__)) arp_packet;
void arp_clear() {
	for(int i = 0; i < ARP_CACHE_SIZE; i++) {
		arp_cache[i].present = 0;
	}
}
static int arp_request(u32_t ip) {
	u8_t *buf = malloc(sizeof(arp_packet) + ETHERNET_HDR);
	CreateEthernetHeader(arp_target, ETHERNET_TYPE_ARP, buf);
	arp_packet *packet = (arp_packet *)(buf + ETHERNET_HDR);
	packet->hw_addr_type = htons(1);
	packet->pr_addr_type = htons(0x800);
	packet->hw_addr_size = 6;
	packet->pr_addr_size = 4;
	packet->operation = htons(1);
	EthernetMyIP(&packet->ip_sender);
	EthernetMyMAC(packet->hw_sender);
	memcpy(packet->hw_target, arp_target, 6);
	packet->ip_target = ip;
	EthernetSent(buf, sizeof(arp_packet) + ETHERNET_HDR);
	free(buf);
	return 0;
}
static int arp_lookup(u32_t ip) {
	for(int i = 0; i < ARP_CACHE_SIZE; i++) {
		if(	(arp_cache[i].present) &&
			(uptime() - arp_cache[i].created <= ARP_TIMEOUT) &&
			(arp_cache[i].ip == ip)) {
			return i;
		}
	}
	return -1;
}
int arp_get_mac(u32_t ip, void *mac) {
	int slot = arp_lookup(ip);
	if(slot >= 0) {
		memcpy(mac, arp_cache[slot].mac, 6);
		return 0;
	}
	arp_request(ip);
	int start = uptime();
	do {
		slot = arp_lookup(ip);
		if(slot >= 0) {
			memcpy(mac, arp_cache[slot].mac, 6);
			return 0;
		}
		sched_yield();
	} while(uptime() - start < 3000);
	return -1;
}
void arp_handle(char *buf) {
	arp_packet *packet = (arp_packet *)(buf + ETHERNET_HDR);
	switch(ntohs(packet->operation)) {
	case 1: { // запрос
		u32_t myip;
		EthernetMyIP(&myip);
		if(packet->ip_target == myip) {
			u8_t *bf = malloc(sizeof(arp_packet) + ETHERNET_HDR);
			CreateEthernetHeader(packet->hw_sender, ETHERNET_TYPE_ARP, bf);
			arp_packet *reply = (arp_packet *)(bf + ETHERNET_HDR);
			reply->hw_addr_type = htons(1);
			reply->pr_addr_type = htons(0x800);
			reply->hw_addr_size = 6;
			reply->pr_addr_size = 4;
			reply->operation = htons(2);
			EthernetMyIP(&reply->ip_sender);
			EthernetMyMAC(reply->hw_sender);
			reply->ip_target = packet->ip_sender;
			memcpy(reply->hw_target, packet->hw_sender, 6);
			EthernetSent(bf, sizeof(arp_packet) + ETHERNET_HDR);
			free(bf);
		}
		break;
	}

	case 2:	{ // ответ
		for(int i = 0; i < ARP_CACHE_SIZE; i++) {
			if(	(!arp_cache[i].present) 		||
			   	(packet->ip_sender == arp_cache[i].ip)	||
				(uptime() - arp_cache[i].created > ARP_TIMEOUT)) {
				arp_cache[i].ip = packet->ip_sender;
				memcpy(arp_cache[i].mac, packet->hw_sender, 6);
				arp_cache[i].present = 1;
				arp_cache[i].created = uptime();
				return;
			}
		}
		arp_cache[0].ip = packet->ip_sender;
		memcpy(arp_cache[0].mac, packet->hw_sender, 6);
		arp_cache[0].present = 1;
		arp_cache[0].created = uptime();
		

		break;
	}
	default:
		printf("Unable to handle arp packet %u\n", ntohs(packet->operation));
	}

}
