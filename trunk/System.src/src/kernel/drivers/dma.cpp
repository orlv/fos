/*
	kernel/drivers/dma.cpp
	Copyright (C) 2004-2006 Oleg Fedorov
*/

#include <types.h>
#include <drivers/dma.h>
#include <io.h>
#include <system.h>

#if 1

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

  cli();			/* disable irq's */

  /* set the mask bit for the channel */
  outportb(0x0a, channel | 4);
  /* clear flipflop */
  outportb(0x0c, 0);
  /* set DMA mode (write+single+r/w) */
  outportb(0x0b, (read ? 0x48 : 0x44) + channel);
  /* set DMA page */
  outportb(dmainfo[channel].page, page);
  /* set DMA offset */
  outportb(dmainfo[channel].offset, offset & 0xff);	/* low unsigned char */
  outportb(dmainfo[channel].offset, offset >> 8);	/* high unsigned char */
  /* set DMA length */
  outportb(dmainfo[channel].length, length & 0xff);	/* low unsigned char */
  outportb(dmainfo[channel].length, length >> 8);	/* high unsigned char */
  /* clear DMA mask bit */
  outportb(0x0a, channel);

  sti();			/* enable irq's */
}
#endif
