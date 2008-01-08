#ifndef ETHERNET_H
#define ETHERNET_H
int CreateEthernetHeader(u8_t *target, u16_t type, void *buf);
int EthernetSent(void *buf, int size);
void EthernetMyIP(void *buf);
void EthernetMyMAC(void *buf);
u16_t EthernetType(void *buf);
int EthernetGetTarget(u32_t ip, u8_t *mac);
#define ETHERNET_HDR		14
#define ETHERNET_TYPE_ARP	0x806
#define ETHERNET_TYPE_IP	0x800
#define ETHERNET_FORMAT_IP(a,b,c,d) (((a & 0xFF) << 0) | ((b & 0xFF) << 8) | ((c & 0xFF) << 16) | ((d & 0xFF) << 24))
#endif
