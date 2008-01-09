#ifndef RTL8139_H
#define RTL8139_H
int rtl8139_init(pci_addr_t *addr);
void rtl8139_transmit();
int rtl8139_poll();
extern u8_t rtl8139_mac[8];
#endif
