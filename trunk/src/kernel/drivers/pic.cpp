/*
  drivers/pic.cpp
  Copyright (C) 2007 Oleg Fedorov

  Основано на коде PIC из StoryOS. (C) 2007 Peter Zotov
*/

#include "pic.h"
#include <fos/hal.h>

PIC::PIC()
{
  remap(0x20, 0x28);
}

void PIC::remap(u8_t v1, u8_t v2)
{
  v1 &= 0xF8;
  v2 &= 0xF8;
  hal->outportb(0x20, 0x11);
  hal->outportb(0xA0, 0x11);
  hal->outportb(0x21, v1);
  hal->outportb(0xA1, v2);
  hal->outportb(0x21, 0x04);
  hal->outportb(0xA1, 0x02);
  hal->outportb(0x21, 0x01);
  hal->outportb(0xA1, 0x01);

  /* запретим прерывания, кроме каскадируемого */
  hal->outportb(0xA1, 0xff);
  hal->outportb(0x21, 0xfb);
}

/* запрещает IRQ номер n */
void PIC::mask(u8_t n)
{
  if (n > 15)
    hal->panic("PIC::mask(): n > 16!");

  u8_t port;
  
  if (n > 7) {
    n -= 8;
    port = 0xA1;
  } else
    port = 0x21;

  hal->outportb(port, hal->inportb(port) | (1<<n));
}

/* разрешает IRQ номер n */
void PIC::unmask(u8_t n)
{
  if(n > 15)
    hal->panic("PIC::unmask(): n > 16!");

  u8_t port;
  
  if (n > 7) {
    n -= 8;
    port = 0xa1;
  } else {
    port = 0x21;
  }

  hal->outportb(port, hal->inportb(port) & ~(1<<n));
}

void PIC::lock()
{
  status = (hal->inportb(0xa1) << 8) | hal->inportb(0x21);
  hal->outportb(0xA1, 0xff);
  hal->outportb(0x21, 0xfb);
}

void PIC::unlock()
{
  hal->outportb(0xa1, status >> 8);
  hal->outportb(0x21, status & 0xff);
}
