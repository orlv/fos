/*
  drivers/char/tty/tty.cpp
  Copyright (C) 2004-2006 Oleg Fedorov
*/

#warning  TODO: Упростить!!!! Выкинуть всё лишнее. 2006-11-12. Oleg.

#include <drivers/block/vga/vga.h>
#include "tty.h"
//#include <vsprintf.h>
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
  mode = TTY_MODE_BLOCK;
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

void TTY::out_ch(const char ch)
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
    if (ch >= 0x20){
      offs++;
      if (offs > bufsize / 2) {
	scroll_up();
	offs -= geom.width;
      }
      buffer[offs - 1] = ch | color;
    }
  }
}

void TTY::set_mode(u32_t mode)
{
  this->mode = mode;
}

#include <stdio.h>

size_t TTY::write(off_t offset, const void *buf, size_t count)
{
  mutex.lock();
  if(mode == TTY_MODE_BLOCK){
    for (size_t i = 0; i < count; i++)
      out_ch(((const char *)buf)[i]);
    refresh();
  } else {
    stdout->write(offset, buf, count);
  }
  mutex.unlock();
  return count;
}

size_t TTY::read(off_t offset, void *buf, size_t count)
{
  if(count + offset > bufsize/2)
    count = bufsize/2 - offset;

  char ch;
  size_t i, j;
  mutex.lock();
  for(i=offset, j=0; (i < (offset + count)) && (i < offs) ; i++){
    ch = buffer[i] & 0xff;
    if(ch){
      ((char *)buf)[j] = ch;
      j++;
    } else if(!((i+1)%geom.width)){
      ((char *)buf)[j] = '\n';
      j++;
    }
  }
  mutex.unlock();
  return j;
}

void TTY::set_text_color(u8_t color)
{
  textcolor = color;
  this->color = (textcolor << 8) | (bgcolor << 16);
}

void TTY::set_bg_color(u8_t color)
{
  bgcolor = color;
  this->color = (textcolor << 8) | (bgcolor << 16);
}
