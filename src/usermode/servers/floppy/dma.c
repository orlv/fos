/*
  kernel/drivers/dma.c
  Copyright (C) 2004-2006 Oleg Fedorov
*/

#include <types.h>
#include <sys/io.h>
#include "dma.h"
const struct dma_channel dmainfo[] = {
  {0x87, 0x00, 0x01},
  {0x83, 0x02, 0x03},
  {0x81, 0x04, 0x05},
  {0x82, 0x06, 0x07}
};


/*
 * this sets up a DMA trasfer between a device and memory.  Pass the DMA
 * channel number (0..3), the physical address of the buffer and transfer
 * length.  If 'read' is TRUE, then transfer will be from memory to device,
 * else from the device to memory.
 */
void dma_xfer(u16_t channel, u32_t physaddr, u16_t length, u8_t read)
{
  u32_t page, offset;

  /* calculate dma page and offset */
  page = physaddr >> 16;
  offset = physaddr & 0xffff;
  length -= 1;			/* with dma, if you want k unsigned chars, you ask for k - 1 */

  //hal->cli();			/* disable irq's */

  /* set the mask bit for the channel */
  outb(channel | 4, 0x0a);
  /* clear flipflop */
  outb(0, 0x0c);
  /* set DMA mode (write+single+r/w) */
  outb((read ? 0x48 : 0x44) + channel, 0x0b);
  /* set DMA page */
  outb(page, dmainfo[channel].page);
  /* set DMA offset */
  outb(offset & 0xff, dmainfo[channel].offset); /* low unsigned char */
  outb(offset >> 8, dmainfo[channel].offset);   /* high unsigned char */
  /* set DMA length */
  outb(length & 0xff, dmainfo[channel].length); /* low unsigned char */
  outb(length >> 8, dmainfo[channel].length);   /* high unsigned char */
  /* clear DMA mask bit */
  outb(channel, 0x0a);
  
  //hal->sti();			/* enable irq's */
}
