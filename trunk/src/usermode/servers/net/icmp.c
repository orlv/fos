#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "endian.h"
#include "nettypes.h"
#include "ip.h"
#include "ethernet.h"
typedef struct {
	u16_t id;
	u16_t seq;
}  __attribute__((__packed__)) icmp_ping;

typedef struct {
	u8_t	type;
	u8_t	code;
	u16_t	checksum;
}  __attribute__((__packed__)) icmp_header;

void icmp_handle(char *buf) {
	ip_packet *ippacket = (ip_packet *)(buf + ETHERNET_HDR);
	icmp_header *packet = (icmp_header *)(buf + ETHERNET_HDR + ippacket->ihl * 4);
	printf("ICMP packet\n");
	printf(" Type: %x\n", packet->type);
	printf(" Code: %x\n", packet->code);
	printf(" Checksum: %x\n", ntohs(packet->checksum));
	switch(packet->type) {
	case 8: {
		icmp_ping *ping  = (icmp_ping *)(buf + ETHERNET_HDR + ippacket->ihl * 4 + sizeof(icmp_header));
		printf(" ICMP ping request\n");
		printf("  ID: %x\n", ntohs(ping->id));
		printf("  Seq: %x\n", ntohs(ping->seq));
		int datalen = ntohs(ippacket->total_len) - sizeof(ip_packet) - sizeof(icmp_header) - sizeof(icmp_ping);
		char *data = buf + ETHERNET_HDR + ippacket->ihl * 4 + sizeof(icmp_header) + sizeof(icmp_ping);
		printf("  Data size: %u bytes\n", datalen);
		char *buf = malloc(sizeof(icmp_header) + sizeof(icmp_ping) + datalen);
		icmp_header *reply_hdr = (icmp_header *) buf;
		reply_hdr->type = 0;
		reply_hdr->code = 0;
		reply_hdr->checksum = ip_checksum(reply_hdr, sizeof(icmp_header));
		icmp_ping *reply_ping = (icmp_ping *)(buf + sizeof(icmp_header));
		reply_ping->id = ping->id;
		reply_ping->seq = ping->seq;
		memcpy(buf + sizeof(icmp_header) + sizeof(icmp_ping), data, datalen);
		ip_send(ippacket->from, IP_PROTOCOL_ICMP, buf, datalen, 0);
		free(buf);
		break;
	}
	default:
		printf("Don't know how to handle icmp packet %x\n", packet->type);
		break;
	}
}
