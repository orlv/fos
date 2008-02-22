/*
 * Copyright (c) 2008 Sergey Gridassov
 * Original code (PCI Configuration Server) (c) 2007  Michael Zhilin
 */

#include <types.h>
#include <sys/pci.h>
#include <sys/io.h>
#include "pci.h"

void poutb(u32_t addr, u8_t reg, u8_t data) {
	outl(addr | (reg & ~3), PCI_CONFIG_CMD);
	outb(data, PCI_CONFIG_DAT + (reg & 3));

}

void poutw(u32_t addr, u8_t reg, u16_t data) {
	outl(addr | (reg & ~3), PCI_CONFIG_CMD);
	outw(data, PCI_CONFIG_DAT + (reg & 3));
}

void poutl(u32_t addr, u8_t reg, u32_t data) {
	outl(addr | (reg & ~3), PCI_CONFIG_CMD);
	outl(data, PCI_CONFIG_DAT + (reg & 3));
}

u8_t pinb(u32_t addr, u8_t reg) {
	outl(addr | (reg & ~3), PCI_CONFIG_CMD);
	return inb(PCI_CONFIG_DAT + (reg & 3));
}

u16_t pinw(u32_t addr, u8_t reg) {
	outl(addr | (reg & ~3), PCI_CONFIG_CMD);
	return inw(PCI_CONFIG_DAT + (reg & 3));
}

u32_t pinl(u32_t addr, u8_t reg) {
	outl(addr | (reg & ~3), PCI_CONFIG_CMD);
	return inl(PCI_CONFIG_DAT + (reg & 3));
}
