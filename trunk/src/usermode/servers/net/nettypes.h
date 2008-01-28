#ifndef NETTYPES_H
#define NETTYPES_H
#include <sys/pci.h>

typedef struct {
	int placeholder;
} net_callbacks_t;
typedef struct dev {
	pci_addr_t *addr;
	void *custom;
	int (*device_init)(net_callbacks_t *, struct dev *);
	int (*device_poll)(struct dev *, int);
	void (*device_transmit)(struct dev *);
	char dev_addr[6];
	int packetlen;
	char *packet;
	u32_t ip;
	u32_t mask;
	u32_t gateway;
} netdev_t;


#define ETH_ZLEN	60
#define ETH_FRAME_LEN	1514

#endif
