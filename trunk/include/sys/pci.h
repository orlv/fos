/*
 * include/sys/pci.h
 * Copyright (c) 2008 Sergey Gridassov
 * Original code (PCI Configuration Server) (c) 2007  Michael Zhilin
 * Переделано в библиотеку.
 */
#ifndef _SYS_PCI_H_
#define _SYS_PCI_H_
#include <types.h>

#if defined(__cplusplus)
extern "C" {
#endif
typedef struct {
	int enable;
	int bus;
	int slot;
	int func;
} pci_addr_t;

int pci_outb(pci_addr_t *addr, int reg, u8_t val);
int pci_outw(pci_addr_t *addr, int reg, u16_t val);
int pci_outl(pci_addr_t *addr, int reg, u32_t val);

int pci_inb(pci_addr_t *addr, int reg);
int pci_inw(pci_addr_t *addr, int reg);
int pci_inl(pci_addr_t *addr, int reg);

int pci_check(pci_addr_t *addr);
#if defined(__cplusplus)
}
#endif
#define PCI_NOTIFY_NEW 0xFFFF00
#endif
