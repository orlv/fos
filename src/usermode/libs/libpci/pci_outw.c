/*
 * Copyright (c) 2008 Sergey Gridassov
 * Original code (PCI Configuration Server) (c) 2007  Michael Zhilin
 * Переделано в библиотеку.
 */
#include <sys/pci.h>
#include <types.h>
#include <sys/io.h>
#include "private.h"
int pci_outw(pci_addr_t *addr, int reg, u16_t val) {
	// FIXME: мутексы!
	u32_t cmd = pci_cmdByItem(addr, reg);
	if(!cmd)
		return -1;
	outl(cmd, PCI_CONFIG_CMD);
	outw(val, PCI_CONFIG_DAT + (reg & 0x03));
	return 0;	
}
