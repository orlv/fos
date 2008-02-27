#ifndef IP_H
#define IP_H
typedef struct {
	int ihl : 4;
	int version :4;
	u8_t type_of_service;
	u16_t total_len;
	u16_t id;
	u16_t frag_off;
	u8_t ttl;
	u8_t protocol;
	u16_t checksum;
	u32_t from;
	u32_t to;
} __attribute__((__packed__)) ip_packet;

void ip_handle(char *buf);
#define IP_PROTOCOL_ICMP	1

u16_t ip_checksum(void *buf, int size);
int ip_send(u32_t to, u8_t protocol, void *data, int datalen, int fragment);
#endif
