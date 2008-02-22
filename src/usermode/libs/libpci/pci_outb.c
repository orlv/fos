/*
 * Copyright (c) 2008 Sergey Gridassov
 * Original code (PCI Configuration Server) (c) 2007  Michael Zhilin
 * Переделано в библиотеку.
 */
#include <sys/pci.h>
#include <unistd.h>
#include "private.h"

int pci_outb(pci_addr_t *addr, int reg, u8_t val) {
	int hndl = pci_open(addr);
	if(hndl < 0)
		return -1;
	lseek(hndl, reg, SEEK_SET);
	write(hndl, &val, 1);
	return 0;	
}
