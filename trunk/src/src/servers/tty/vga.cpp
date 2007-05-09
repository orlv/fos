/*
  apps/tty/vga.cpp
  Copyright (C) 2004-2007 Oleg Fedorov

  (Tue Mar 13 21:04:41 2007) Доработано для использования в сервере консоли
*/

#include "vga.h"
#include <io.h>
#include <stdio.h>
#include <fos.h>

VGA::VGA()
{				/*:Tinterface() */
  /* empty */
}

res_t VGA::init()
{
  offs = 0;

  if (!(fb = (u16_t *) kmemmap(0xb8000, 0x1000)))
    return RES_FAULT;

  geom.height = 25;
  geom.width = 80;
  bufsize = geom.height * geom.width;

  reset();
  //return RES_SUCCESS;
  return (u32_t) fb;
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
  //  SetCursorType(_NOCURSOR);
  //  MoveCursor(0);
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
  outportb(VGA_CRT_IC, 0x0f);
  outportb(VGA_CRT_DC, offset & 0xff);
  offset >>= 8;
  outportb(VGA_CRT_IC, 0x0e);
  outportb(VGA_CRT_DC, offset & 0xff);
}

void VGA::SetCursorType(unsigned char show)
{
  if (show) {
    outportb(VGA_CRT_IC, 0x0a);
    outportb(VGA_CRT_DC, 12);
    outportb(VGA_CRT_IC, 0x0b);
    outportb(VGA_CRT_DC, 13);
  } else {
    outportb(VGA_CRT_IC, 0x0a);
    outportb(VGA_CRT_DC, 32);
    outportb(VGA_CRT_IC, 0x0b);
    outportb(VGA_CRT_DC, 32);
  }
}

void VGA::SetFont(unsigned char *fnt)
{
  int i = 0, j = 0;
  unsigned char *charmap;
  charmap = ((unsigned char *)0xa0000);
  outportb(VGA_CRT_IC, 0x00);	/* First, the sequencer */
  outportb(VGA_CRT_DC, 0x01);	/* Synchronous reset */
  outportb(VGA_CRT_IC, 0x02);
  outportb(VGA_CRT_DC, 0x04);	/* CPU writes only to map 2 */
  outportb(VGA_CRT_IC, 0x04);
  outportb(VGA_CRT_DC, 0x07);	/* Sequential addressing */
  outportb(VGA_CRT_IC, 0x00);
  outportb(VGA_CRT_DC, 0x03);	/* Clear synchronous reset */

  outportb(VGA_GFX_I, 0x04);	/* Now, the graphics controller */
  outportb(VGA_GFX_D, 0x02);	/* select map 2 */
  outportb(VGA_GFX_I, 0x05);
  outportb(VGA_GFX_D, 0x00);	/* disable odd-even addressing */
  outportb(VGA_GFX_I, 0x06);
  outportb(VGA_GFX_D, 0x00);	/* map start at A000:0000 */

  /* скопируем шрифт */
  while (i < 0x1000) {
    charmap[j++] = fnt[i++];
    if (!(j % 16)) {
      while (((j + 1) % 16))
	charmap[j++] = 0;
      j++;
    }
  }

  outportb(VGA_CRT_IC, 0x00);	/* Frist, the sequencer */
  outportb(VGA_CRT_DC, 0x01);	/* Synchronous reset */
  outportb(VGA_CRT_IC, 0x02);
  outportb(VGA_CRT_DC, 0x03);	/* CPU writes to maps 0 and 1 */
  outportb(VGA_CRT_IC, 0x04);
  outportb(VGA_CRT_DC, 0x03);	/* odd-even addressing */
  outportb(VGA_CRT_IC, 0x00);
  outportb(VGA_CRT_DC, 0x03);	/* clear synchronous reset */

  outportb(VGA_GFX_I, 0x04);	/* Now, the graphics controller */
  outportb(VGA_GFX_D, 0x00);	/* select map 0 for CPU */
  outportb(VGA_GFX_I, 0x05);
  outportb(VGA_GFX_D, 0x10);	/* enable even-odd addressing */
  outportb(VGA_GFX_I, 0x06);
  outportb(VGA_GFX_D, 0x0e);	/* map starts at b800:0 or b000:0 */

  //printf("Шрифт установлен.\n");
}
