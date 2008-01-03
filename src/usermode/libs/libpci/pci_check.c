/*
 * Copyright (c) 2008 Sergey Gridassov
 * Original code (PCI Configuration Server) (c) 2007  Michael Zhilin
 */
#include <sys/pci.h>
#include <types.h>
#include <sys/io.h>
#include "private.h"
int pci_check(pci_addr_t *addr) {
	// FIXME: мутексы!
	u32_t res = inl(PCI_CONFIG_CMD);
	
	u32_t root_cmd = pci_cmdByItem(addr, 0);
	if(!root_cmd)
		return -1;

	outl(root_cmd, PCI_CONFIG_CMD);

	res = inl(PCI_CONFIG_DAT);
	if(res !=  0xffffffff)
		return 0;
	return -1;
}

