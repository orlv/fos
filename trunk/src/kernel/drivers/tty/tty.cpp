/*
  drivers/char/tty/tty.cpp
  Copyright (C) 2004-2007 Oleg Fedorov
*/

#include "tty.h"
#include <string.h>
#include <fos/system.h>

#ifdef DEBUG_SERIAL

#define OutPortByte system->outportb
#define InPortByte system->inportb
#define BASE 0x3f8
#define DLL		0
#define DLM		1
#define RBR		0
#define THR		0
#define IER		1
#define IIR		2
#define LSR		5
#define FCR		8
#define LCR		3
// флаги в IER
#define INTERRUPT_DISABLE	0
#define RECEIVE_ENABLE		1
#define TRANSMIT_ENABLE		2
#define RECEIVER_LINE_ST_ENABLE	4
#define MODEM_ENABLE		8
// флаги в LCR
#define DLAB	0x80
#define Wls8	0x03

#define DR	0x01
#define THRE	0x20

#define IP	0x01

#endif /* QEMU_DEBUG */


TTY::TTY(u16_t width, u16_t height)
{
#ifdef DEBUG_SERIAL
  OutPortByte(BASE + FCR, 0x84);
  int bgc = (1843200 + 8 * 9600 - 1) / (16 * 9600);
  
  OutPortByte(BASE + LCR, DLAB);
  OutPortByte(BASE + DLM, bgc >> 8);
  OutPortByte(BASE + DLL, bgc);
  OutPortByte(BASE + LCR, 0);
  
  OutPortByte(BASE + LCR,  Wls8);
#endif
  geom.width = width;
  geom.height = height;

  bufsize = geom.width * geom.height * 2;

  fb = (u16_t *) 0x000b8000;
  buffer = new u16_t[bufsize];

  for(size_t i=0; i<bufsize; i++)
    buffer[i] = fb[i] = 0;

  textcolor = GREEN;
  bgcolor = BLACK;
  color = (textcolor << 8) | (bgcolor << 12);

  offs = 0;
}

TTY::~TTY()
{
  delete buffer;
}

void TTY::scroll_up()
{
  off_t i;
  for (i = 0; i < geom.width * (geom.height - 1); i++) {
    fb[i] = buffer[i] = buffer[i + geom.width];
  }
  for (; i < geom.width * geom.height; i++) {
    fb[i] = buffer[i] = 0;
  }
}

void TTY::out_ch(const char ch)
{
#ifdef DEBUG_SERIAL
  while(!(InPortByte(BASE + LSR) &THRE)); // ждем опустошения буфера
  OutPortByte(BASE + THR, ch);
#endif

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
      fb[offs - 1] = buffer[offs - 1] = ch | color;
    }
  }
}

size_t TTY::write(off_t offset, const void *buf, size_t count)
{
  for (size_t i = 0; i < count; i++)
    out_ch(((const char *)buf)[i]);
  sync();
  return count;
}

size_t TTY::read(off_t offset, void *buf, size_t count)
{
  if(count + offset > bufsize/2)
    count = bufsize/2 - offset;

  char ch;
  size_t i, j;

  for (i=offset, j=0; (i < (offset + count)) && (i < offs) ; i++) {
    ch = buffer[i] & 0xff;
    if(ch) {
      ((char *)buf)[j] = ch;
      j++;
    } else if(!((i+1)%geom.width)) {
      ((char *)buf)[j] = '\n';
      j++;
    }
  }

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

void TTY::sync()
{
  for(size_t i=0; i<bufsize; i++)
    fb[i] = buffer[i];
}
