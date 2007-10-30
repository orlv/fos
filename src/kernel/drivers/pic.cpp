/*
  drivers/pic.cpp
  Copyright (C) 2007 Oleg Fedorov
*/

#include "pic.h"
#include <fos/system.h>

PIC::PIC()
{
  remap(0x20, 0x28);
}

void PIC::remap(u8_t v1, u8_t v2)
{
  v1 &= 0xF8;
  v2 &= 0xF8;
  system->outportb(0x20, 0x11);
  system->outportb(0xA0, 0x11);
  system->outportb(0x21, v1);
  system->outportb(0xA1, v2);
  system->outportb(0x21, 0x04);
  system->outportb(0xA1, 0x02);
  system->outportb(0x21, 0x01);
  system->outportb(0xA1, 0x01);

  /* запретим прерывания, кроме каскадируемого */
  system->outportb(0xA1, 0xff);
  system->outportb(0x21, 0xfb);
}

/* запрещает IRQ номер n */
void PIC::mask(u8_t n)
{
  if (n > 15)
    system->panic("PIC::mask(): n > 16!");

  u8_t port;
  
  if (n > 7) {
    n -= 8;
    port = 0xA1;
  } else
    port = 0x21;

  system->outportb(port, system->inportb(port) | (1<<n));
}

/* разрешает IRQ номер n */
void PIC::unmask(u8_t n)
{
  if(n > 15)
    system->panic("PIC::unmask(): n > 16!");

  u8_t port;
  
  if (n > 7) {
    n -= 8;
    port = 0xa1;
  } else {
    port = 0x21;
  }

  system->outportb(port, system->inportb(port) & ~(1<<n));
}

void PIC::lock()
{
  status = (system->inportb(0xa1) << 8) | system->inportb(0x21);
  system->outportb(0xA1, 0xff);
  system->outportb(0x21, 0xfb);
}

void PIC::unlock()
{
  system->outportb(0xa1, status >> 8);
  system->outportb(0x21, status & 0xff);
}
