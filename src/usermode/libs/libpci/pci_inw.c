/*
 * Copyright (c) 2008 Sergey Gridassov
 * Original code (PCI Configuration Server) (c) 2007  Michael Zhilin
 * Переделано в библиотеку.
 */
#include <sys/pci.h>
#include <unistd.h>
#include "private.h"

int pci_inw(pci_addr_t *addr, int reg) {
	int hndl = pci_open(addr);
	if(hndl < 0)
		return -1;
	lseek(hndl, reg, SEEK_SET);
	u16_t res;
	int ret = read(hndl, &res, 2);
	close(hndl);
	if(ret < 0)
		return -1;
	return res;
	
}
