/*
 * Copyright (c) 2008 Sergey Gridassov
 * Original code (PCI Configuration Server) (c) 2007  Michael Zhilin
 * Переделано в библиотеку.
 */
#ifndef PRIVATE_H
#define PRIVATE_H
#define	PCI_BUS_ID_MAX	255 	
#define	PCI_SLOT_ID_MAX	31
#define	PCI_FUNC_ID_MAX	7
#define	PCI_REG_ID_MAX	255

#define PCI_CONFIG_CMD 0x0cf8
#define PCI_CONFIG_DAT 0x0cfc

int pci_open(pci_addr_t *addr);

#endif
