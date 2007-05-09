/*
	drivers/block/vga.cpp
	Copyright (C) 2004-2006 Oleg Fedorov
*/

#include "vga.h"
#include <hal.h>

VGA::VGA():Tinterface()
{
  offs = 0;

  fb = (u16_t *) 0x000b8000;
  geom.height = 25;
  geom.width = 80;
  bufsize = geom.height * geom.width;

  info.type = FTypeObject;

  reset();
}

void VGA::scroll_up()
{
  off_t i;
  for (i = 0; i < geom.width * (geom.height - 1); i++) {
    fb[i] = fb[i + geom.width];
  }
  for (; i < geom.width * geom.height; i++) {
    fb[i] = 0;
  }
}

void VGA::OutRaw(u16_t ch)
{
  if (offs > bufsize) {
    scroll_up();
    offs -= geom.width;
  }
  fb[offs] = ch;
  offs++;
}

size_t VGA::read(off_t offset, void *buf, size_t count)
{
#warning TODO: VGA::read() тоже можно бы реализовать.. 2006-11-12. Oleg.
  return 0;
}

size_t VGA::write(off_t offset, const void *buf, size_t count)
{
  if (offset != offs) {
    if (offset < bufsize) {
      offs = offset;
    } else {
      offs = bufsize - 1;
    }
  }

  count /= 2;

  for (size_t c = 0; c < count; c++) {
    OutRaw(((u16_t *) buf)[c]);
  }
  return count;
}

void VGA::reset()
{
  off_t i = geom.height * geom.width;
  SetCursorType(_NOCURSOR);
  MoveCursor(0);
  offs = 0;
  while (i--) {
    OutRaw(0);
  }
  offs = 0;
}

void VGA::MoveCursorXY(u8_t x, u8_t y)
{
  off_t offset = x + y * geom.width;
  MoveCursor(offset);
}

void VGA::MoveCursor(off_t offset)
{
  hal->outportb(VGA_CRT_IC, 0x0f);
  hal->outportb(VGA_CRT_DC, offset & 0xff);
  offset >>= 8;
  hal->outportb(VGA_CRT_IC, 0x0e);
  hal->outportb(VGA_CRT_DC, offset & 0xff);
}

void VGA::SetCursorType(unsigned char show)
{
  if (show) {
    hal->outportb(VGA_CRT_IC, 0x0a);
    hal->outportb(VGA_CRT_DC, 12);
    hal->outportb(VGA_CRT_IC, 0x0b);
    hal->outportb(VGA_CRT_DC, 13);
  } else {
    hal->outportb(VGA_CRT_IC, 0x0a);
    hal->outportb(VGA_CRT_DC, 32);
    hal->outportb(VGA_CRT_IC, 0x0b);
    hal->outportb(VGA_CRT_DC, 32);
  }
}

void VGA::SetFont(unsigned char *fnt)
{
  int i = 0, j = 0;
  unsigned char *charmap;
  charmap = ((unsigned char *)0xa0000);
  hal->outportb(VGA_CRT_IC, 0x00);	/* First, the sequencer */
  hal->outportb(VGA_CRT_DC, 0x01);	/* Synchronous reset */
  hal->outportb(VGA_CRT_IC, 0x02);
  hal->outportb(VGA_CRT_DC, 0x04);	/* CPU writes only to map 2 */
  hal->outportb(VGA_CRT_IC, 0x04);
  hal->outportb(VGA_CRT_DC, 0x07);	/* Sequential addressing */
  hal->outportb(VGA_CRT_IC, 0x00);
  hal->outportb(VGA_CRT_DC, 0x03);	/* Clear synchronous reset */

  hal->outportb(VGA_GFX_I, 0x04);	/* Now, the graphics controller */
  hal->outportb(VGA_GFX_D, 0x02);	/* select map 2 */
  hal->outportb(VGA_GFX_I, 0x05);
  hal->outportb(VGA_GFX_D, 0x00);	/* disable odd-even addressing */
  hal->outportb(VGA_GFX_I, 0x06);
  hal->outportb(VGA_GFX_D, 0x00);	/* map start at A000:0000 */

  /* скопируем шрифт */
  while (i < 0x1000) {
    charmap[j++] = fnt[i++];
    if (!(j % 16)) {
      while (((j + 1) % 16))
	charmap[j++] = 0;
      j++;
    }
  }

  hal->outportb(VGA_CRT_IC, 0x00);	/* Frist, the sequencer */
  hal->outportb(VGA_CRT_DC, 0x01);	/* Synchronous reset */
  hal->outportb(VGA_CRT_IC, 0x02);
  hal->outportb(VGA_CRT_DC, 0x03);	/* CPU writes to maps 0 and 1 */
  hal->outportb(VGA_CRT_IC, 0x04);
  hal->outportb(VGA_CRT_DC, 0x03);	/* odd-even addressing */
  hal->outportb(VGA_CRT_IC, 0x00);
  hal->outportb(VGA_CRT_DC, 0x03);	/* clear synchronous reset */

  hal->outportb(VGA_GFX_I, 0x04);	/* Now, the graphics controller */
  hal->outportb(VGA_GFX_D, 0x00);	/* select map 0 for CPU */
  hal->outportb(VGA_GFX_I, 0x05);
  hal->outportb(VGA_GFX_D, 0x10);	/* enable even-odd addressing */
  hal->outportb(VGA_GFX_I, 0x06);
  hal->outportb(VGA_GFX_D, 0x0e);	/* map starts at b800:0 or b000:0 */

  //printf("Шрифт установлен.\n");
}
