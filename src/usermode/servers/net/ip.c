#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nettypes.h"
#include "ethernet.h"
#include "endian.h"
#include "ip.h"
#include "icmp.h"
static int id = 0;
void ip_handle(char *buf) {
	ip_packet *packet = (ip_packet *) (buf + ETHERNET_HDR);
	if((ntohs(packet->frag_off) & 0x1FFF) > 0 	||
		(ntohs(packet->frag_off) >> 12)	 & 2) {
		printf("%%%%% Sorry, but this IP packet is fragmented. I can't decode it.\n");
		return;
	}
/*	printf("!> IP packet\n");

	printf(" Version: %u\n", packet->version);
	printf(" Header size: %u bytes\n", packet->ihl * 4);

	printf(" Type of service: %x\n", packet->type_of_service);
	printf(" Total length: %u bytes\n", ntohs(packet->total_len));
	printf(" Packet ID: %u\n", ntohs(packet->id));
	printf(" Flags: %x\n", ntohs(packet->frag_off) >> 12);
	printf(" Fragment offset: %u\n", ntohs(packet->frag_off) & 0x1FFF);
	printf(" Time to live: %u\n", packet->ttl);
	printf(" Protocol: %u\n", packet->protocol);
	printf(" Checksum: %u\n", ntohs(packet->checksum));
	printf(" From: %x\n", ntohs(packet->from));
	printf(" To: %x\n", ntohs(packet->to));
*/
	switch(packet->protocol) {
	case IP_PROTOCOL_ICMP:
		icmp_handle(buf);
		break;
	default:
		printf("Can't handle IP packet with type %x\n", packet->protocol);
		break;
	}
} 

u16_t ip_checksum( void *data, int size)
{
	u8_t *dptr;
	size_t n;
	u16_t word;
	u32_t sum;
	int swap= 0;

	sum= 0;
	dptr= data;
	n= size;

	swap= ((size_t) dptr & 1);
	if (swap) {
		sum= ((sum & 0xFF) << 8) | ((sum & 0xFF00) >> 8);
		if (n > 0) {
			((u8_t *) &word)[0]= 0;
			((u8_t *) &word)[1]= dptr[0];
			sum+= (u32_t) word;
			dptr+= 1;
			n-= 1;
		}
	}

	while (n >= 8) {
		sum+= (u32_t) ((u16_t *) dptr)[0]
		    + (u32_t) ((u16_t *) dptr)[1]
		    + (u32_t) ((u16_t *) dptr)[2]
		    + (u32_t) ((u16_t *) dptr)[3];
		dptr+= 8;
		n-= 8;
	}

	while (n >= 2) {
		sum+= (u32_t) ((u16_t *) dptr)[0];
		dptr+= 2;
		n-= 2;
	}

	if (n > 0) {
		((u8_t *) &word)[0]= dptr[0];
		((u8_t *) &word)[1]= 0;
		sum+= (u32_t) word;
	}

	sum= (sum & 0xFFFF) + (sum >> 16);
	if (sum > 0xFFFF) sum++;

	if (swap) {
		sum= ((sum & 0xFF) << 8) | ((sum & 0xFF00) >> 8);
	}
	return sum;
}

int ip_send(u32_t to, u8_t protocol, void *data, int datalen, int fragment) {
	u8_t mac[6];
	if(arp_get_mac(to, mac) < 0) 
		return -1;
	
	char *buf = malloc(ETHERNET_HDR + sizeof(ip_packet) + datalen);
	if(CreateEthernetHeader(mac, ETHERNET_TYPE_IP, buf) < 0) {
		free(buf);
		return -1;
	}
	ip_packet *packet = (ip_packet *) (buf + ETHERNET_HDR);
	packet->ihl = sizeof(ip_packet) / 4;
	packet->version = 4;
	packet->type_of_service = 0;
	packet->total_len = htons(sizeof(ip_packet) + datalen);
	id++;
	packet->id = htons(id);
	packet->frag_off = htons(0 | ((!fragment ? 4:0) << 12));
	packet->ttl = 64;
	packet->protocol = protocol;
	packet->to = to;
	EthernetMyIP(&packet->from);
	packet->checksum = ip_checksum(packet, sizeof(ip_packet));
	memcpy(buf + ETHERNET_HDR + sizeof(ip_packet), data, datalen);
	printf("IP sending %u bytes\n", ETHERNET_HDR + sizeof(ip_packet) + datalen);
	EthernetSent(buf, ETHERNET_HDR + sizeof(ip_packet) + datalen);
	free(buf);
	return 0;
}
