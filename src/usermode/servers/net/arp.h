#ifndef ARP_H
#define ARP_H
int arp_get_mac(u32_t ip, void *mac);
void arp_handle(char *buf);
void arp_clear();
#endif
