/*
  drivers/pic.cpp
  Copyright (C) 2007 Peter Zotov
  Copyright (C) 2007 Oleg Fedorov
*/

#include "pic.h"
#include <hal.h>

PIC::PIC()
{
  int i;
  /* запретим IRQ 0-15 */
  for(i = 0; i < 16; i++){
    mask(i);
  }
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
  hal->outportb(0x21, 0x0);
  hal->outportb(0xA1, 0x0);
}

void PIC::mask(u8_t n)
{
  if(n > 15)
    hal->panic("PIC::mask(): n > 16!");
  if(n == 2)
    n = 9;
  bool second_pic = n > 7;
  if(second_pic)
    n -= 8;
  u8_t byte = 1 << n;
  u8_t port = second_pic ? 0xA1 : 0x21;
  u8_t mask = hal->inportb(port);
  mask = mask | byte;
  hal->outportb(port, mask);
}

void PIC::unmask(u8_t n)
{
  if(n > 15)
    hal->panic("PIC::unmask(): n > 16!");
  bool second_pic = n > 7;
  if(second_pic)
    n -= 8;
  u8_t byte = 1 << n;
  u8_t port = second_pic ? 0xA1 : 0x21;
  u8_t mask = hal->inportb(port);
  mask = mask & (~byte);
  hal->outportb(port, mask);
}
