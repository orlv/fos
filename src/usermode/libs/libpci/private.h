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

static inline u32_t pci_cmdByItem(pci_addr_t *addr, int reg) {
	if((addr->bus <= PCI_BUS_ID_MAX )	&&
	(addr->slot <= PCI_SLOT_ID_MAX) 	&&
	(addr->func <= PCI_FUNC_ID_MAX)	&&
	(reg <= PCI_REG_ID_MAX))
		return (addr->enable << 31) 	|
		(addr->bus << 16)		|
		(addr->slot << 11) 		|
		(addr->func << 8)		|
		(reg & ~0x03);
	return 0;
}

#endif
