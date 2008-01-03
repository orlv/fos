/*
 * Copyright (c) 2008 Sergey Gridassov
 * Original code (PCI Configuration Server) (c) 2007  Michael Zhilin
 * Переделано в библиотеку.
 */
#include <sys/pci.h>
#include <types.h>
#include <sys/io.h>
#include "private.h"
int pci_inl(pci_addr_t *addr, int reg) {
	// FIXME: мутексы!
	u32_t cmd = cmdByItem(addr, reg);
	if(!cmd)
		return -1;
	outl(cmd, PCI_CONFIG_CMD);
	return inl(PCI_CONFIG_DAT + (reg & 0x03));
}
