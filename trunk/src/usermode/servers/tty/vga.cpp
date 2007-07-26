/*
  apps/tty/vga.cpp
  Copyright (C) 2004-2007 Oleg Fedorov

  (Tue Mar 13 21:04:41 2007) Доработано для использования в сервере консоли
*/

#include <fos/page.h>
#include <sys/io.h>
#include <stdio.h>
#include "vga.h"

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
  outb(0x0f, VGA_CRT_IC);
  outb(offset & 0xff, VGA_CRT_DC);
  offset >>= 8;
  outb(0x0e, VGA_CRT_IC);
  outb(offset & 0xff, VGA_CRT_DC);
}

void VGA::SetCursorType(unsigned char show)
{
  if (show) {
    outb(0x0a, VGA_CRT_IC);
    outb(12, VGA_CRT_DC);
    outb(0x0b, VGA_CRT_IC);
    outb(13, VGA_CRT_DC);
  } else {
    outb(0x0a, VGA_CRT_IC);
    outb(32, VGA_CRT_DC);
    outb(0x0b, VGA_CRT_IC);
    outb(32, VGA_CRT_DC);
  }
}

void VGA::SetFont(unsigned char *fnt)
{
  int i = 0, j = 0;
  unsigned char *charmap = ((unsigned char *)0xa0000);
  outb(0x00, VGA_CRT_IC);	/* First, the sequencer */
  outb(0x01, VGA_CRT_DC);	/* Synchronous reset */
  outb(0x02, VGA_CRT_IC);
  outb(0x04, VGA_CRT_DC);	/* CPU writes only to map 2 */
  outb(0x04, VGA_CRT_IC);
  outb(0x07, VGA_CRT_DC);	/* Sequential addressing */
  outb(0x00, VGA_CRT_IC);
  outb(0x03, VGA_CRT_DC);	/* Clear synchronous reset */

  outb(0x04, VGA_GFX_I);	/* Now, the graphics controller */
  outb(0x02, VGA_GFX_D);	/* select map 2 */
  outb(0x05, VGA_GFX_I);
  outb(0x00, VGA_GFX_D);	/* disable odd-even addressing */
  outb(0x06, VGA_GFX_I);
  outb(0x00, VGA_GFX_D);	/* map start at A000:0000 */

  /* скопируем шрифт */
  while (i < 0x1000) {
    charmap[j++] = fnt[i++];
    if (!(j % 16)) {
      while (((j + 1) % 16))
	charmap[j++] = 0;
      j++;
    }
  }

  outb(0x00, VGA_CRT_IC);	/* Frist, the sequencer */
  outb(0x01, VGA_CRT_DC);	/* Synchronous reset */
  outb(0x02, VGA_CRT_IC);
  outb(0x03, VGA_CRT_DC);	/* CPU writes to maps 0 and 1 */
  outb(0x04, VGA_CRT_IC);
  outb(0x03, VGA_CRT_DC);	/* odd-even addressing */
  outb(0x00, VGA_CRT_IC);
  outb(0x03, VGA_CRT_DC);	/* clear synchronous reset */

  outb(0x04, VGA_GFX_I);	/* Now, the graphics controller */
  outb(0x00, VGA_GFX_D);	/* select map 0 for CPU */
  outb(0x05, VGA_GFX_I);
  outb(0x10, VGA_GFX_D);	/* enable even-odd addressing */
  outb(0x06, VGA_GFX_I);
  outb(0x0e, VGA_GFX_D);	/* map starts at b800:0 or b000:0 */

  //printf("Шрифт установлен.\n");
}
