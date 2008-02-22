/*
 * Copyright (c) 2008 Sergey Gridassov
 * Original code (PCI Configuration Server) (c) 2007  Michael Zhilin
 */
#include <sys/pci.h>
#include <unistd.h>
#include "private.h"
int pci_check(pci_addr_t *addr) {
	
	int root_cmd = pci_open(addr);
	if(root_cmd < 0)
		return -1;

	u32_t res;
	lseek(res, 0, SEEK_SET);
	int ret = read(root_cmd, &res, 4);
	close(root_cmd);
	return -1;
	if(ret < 4) 
		return -1;

	if(res ==  0xffffffff)
		return -1;
	return 0;
}

