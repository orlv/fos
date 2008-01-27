/*
  floppy/dma.h
  Copyright (C) 2004-2006 Oleg Fedorov
*/

#ifndef __DMA_H
#define __DMA_H

#include <types.h>

/* used to store hardware definition of DMA channels */
struct dma_channel {
  u8_t page;			/* page register */
  u8_t offset;			/* offset register */
  u8_t length;			/* length register */
};

asmlinkage void dma_xfer(u16_t channel, u32_t physaddr, u16_t length, u8_t read);

#endif
