/*
  drivers/char/tty/tty.cpp
  Copyright (C) 2004-2006 Oleg Fedorov
*/

#warning  TODO: Упростить!!!! Выкинуть всё лишнее. 2006-11-12. Oleg.

#include <drivers/block/vga/vga.h>
#include "tty.h"
#include <vsprintf.h>
#include <string.h>

TTY::TTY(u16_t width, u16_t height):Tinterface()
{
  geom.width = width;
  geom.height = height;

  bufsize = geom.width * geom.height * 2;

  buffer = new u16_t[geom.width * geom.height];

  textcolor = GREEN;
  bgcolor = BLACK;
  color = (textcolor << 8) | (bgcolor << 16);

  offs = 0;

  info.type = FTypeObject;
}

TTY::~TTY()
{
  delete buffer;
}

void TTY::scroll_up()
{
  off_t i;
  for (i = 0; i < geom.width * (geom.height - 1); i++) {
    buffer[i] = buffer[i + geom.width];
  }
  for (; i < geom.width * geom.height; i++) {
    buffer[i] = 0;
  }
}

void TTY::refresh()
{
  if (stdout)			/* stdout (например vga) можно отключить, и подключить 
				   к нему другую виртуальную консоль */
    stdout->write(0, buffer, bufsize);
}

void TTY::Out(const char ch)
{
  switch (ch) {
  case '\n':
    offs += geom.width;
    if (offs > bufsize / 2) {
      scroll_up();
      offs -= geom.width;
    }

  case '\r':
    offs -= offs % geom.width;
    break;

  default:
    if (ch >= 0x20)
      OutRaw(ch);
  }
}

void TTY::OutRaw(const u8_t ch)
{
  offs++;
  if (offs > bufsize / 2) {
    scroll_up();
    offs -= geom.width;
  }
  buffer[offs - 1] = ch | color;
}

void TTY::outs(const char *str)
{
  size_t len = strlen(str);
  for (size_t i = 0; i < len; i++)
    Out(str[i]);
  refresh();
}

void TTY::outs(const char *str, size_t len)
{
  for (size_t i = 0; i < len; i++)
    Out(str[i]);
  refresh();
}

size_t TTY::read(off_t offset, void *buf, size_t count)
{
  return 0;
}

size_t TTY::write(off_t offset, const void *buf, size_t count)
{
  outs((const char *)buf, count);
  return count;
}

/* это тут вообще для прикола */
TTY & TTY::operator <<(const char *str)
{
  outs(str);
  return (*this);
}

TTY & TTY::operator <<(unsigned int t)
{
  size_t i;
  //char *str = new char[11];
  char str[11];

  i = sprintf(str, "%d", t);
  outs(str, i);

  //delete str;
  return (*this);
}

void TTY::SetTextColor(u8_t tcolor)
{
  textcolor = tcolor;
  color = (textcolor << 8) | (bgcolor << 16);
}

void TTY::SetBgColor(u8_t tcolor)
{
  bgcolor = tcolor;
  color = (textcolor << 8) | (bgcolor << 16);
}
