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
	char dev_addr[6];
} dev_t;
#endif
