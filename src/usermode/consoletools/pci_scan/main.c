/*
 * Copyright (c) 2008 Sergey Gridassov
 * Original code (PCI Configuration Server) (c) 2007  Michael Zhilin
 */

#include "config.h"
#include "scan.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
	parse_config();
	for(int i = 0; i <= PCI_BUS_ID_MAX; i++) {
		scan_pci(i);
	}
	printf("PCI: Scan completed\n");
	return 0;
} 
