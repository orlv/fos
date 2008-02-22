 
/*
 * Copyright (c) 2008 Sergey Gridassov
 * Original code (PCI Configuration Server) (c) 2007  Michael Zhilin
 */
#include <stdio.h>
#include <sys/pci.h>
#include "scan.h"
#include "config.h"
int get_dev_info(pci_addr_t *addr, dev_info *info) {
	addr->enable = 1;
	info->device_id = pci_inw(addr, PCI_CONF_DEVICE_OFFSET);
	if(info->device_id == 0 || info->device_id == PCI_CONF_INVALID_DEVICE)
		return -1;

	info->vendor_id = pci_inw(addr, PCI_CONF_VENDOR_OFFSET);
	info->class_id = pci_inl(addr, PCI_CONF_REVISION_OFFSET) >> 8;
	return 0;
}
void scan_pci(int bus) {
	pci_addr_t node;
	dev_info info;
	node.enable = 1;
	node.slot = 0;
	node.bus = bus;
	node.func = 0;
	if(pci_inw(&node, PCI_CONF_DEVICE_OFFSET) == PCI_CONF_INVALID_DEVICE) return;
	for(int i = 0; i <= PCI_SLOT_ID_MAX; i++) {
		node.slot = i;
		node.func = 0;
		if(get_dev_info(&node, &info) < 0)
			continue;

		printf("PCI: %u.%u: %s DID: %04x VID: %04x\n", bus, i, find_device(info.class_id), info.device_id, info.vendor_id);
		TryHandleDevice(info.device_id, info.vendor_id, &node);
		for(int j = 1; j <= PCI_FUNC_ID_MAX; j++) {
			node.func = j;
			if(get_dev_info(&node, &info) < 0)
				continue;;
			printf("PCI: %u.%u.%u: %s DID:%04x VID:%04x\n", bus, i, j, find_device(info.class_id), info.device_id, info.vendor_id);
			TryHandleDevice(info.device_id, info.vendor_id, &node);
		}
	}
}
