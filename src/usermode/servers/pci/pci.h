/*
 * Copyright (c) 2008 Sergey Gridassov
 * Original code (PCI Configuration Server) (c) 2007  Michael Zhilin
 */

#ifndef PCI_H
#define PCI_H
#define	PCI_BUS_ID_MAX	255 	
#define	PCI_SLOT_ID_MAX	31
#define	PCI_FUNC_ID_MAX	7
#define	PCI_REG_ID_MAX	255
#define PCI_CONFIG_CMD 0x0cf8
#define PCI_CONFIG_DAT 0x0cfc

void poutb(u32_t addr, u8_t reg, u8_t data);
void poutw(u32_t addr, u8_t reg, u16_t data);
void poutl(u32_t addr, u8_t reg, u32_t data);

u8_t pinb(u32_t addr, u8_t reg);
u16_t pinw(u32_t addr, u8_t reg);
u32_t pinl(u32_t addr, u8_t reg);

#endif
