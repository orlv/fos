/*
 * Copyright (c) 2008 Sergey Gridassov
 * Original code (PCI Configuration Server) (c) 2007  Michael Zhilin
 * Переделано в библиотеку.
 */
#include <sys/pci.h>
#include <types.h>
#include <sys/io.h>
#include "private.h"
int pci_outl(pci_addr_t *addr, int reg, u32_t val) {
	// FIXME: мутексы!
	u32_t cmd = cmdByItem(addr, reg);
	if(!cmd)
		return -1;
	outl(cmd, PCI_CONFIG_CMD);
	outl(val, PCI_CONFIG_DAT + (reg & 0x03));
	return 0;	
}
