/*
  drivers/char/keyboard/keyboard.cpp
  Copyright (C) 2004-2006 Oleg Fedorov
*/

#include "keyboard.h"
#include <io.h>
#include <system.h>
#include <stdio.h>

#define PORT_KBD_A      0x60

#define ENABLE_KEYBOARD_IRQ()	outportb(0x21, inportb(0x21) & 0xfd  )

Keyboard::Keyboard()
{
  buffer = new u8_t[KB_KEYS_BUFF_SIZE];

  ENABLE_KEYBOARD_IRQ();
  SetRepeatRate(0);
  led_update();
  printk("\nKeyboard enabled");
}

Keyboard::~Keyboard()
{
  delete buffer;
}

volatile void Keyboard::SetRepeatRate(u8_t rate)
{
  wait();
  outportb(0x60, 0xf3);
  wait();
  outportb(0x60, rate);
}

volatile void Keyboard::handler()
{
  u8_t scancode;
  wait();
  outportb(0x64, 0xad);		/* disable keyboard */
  wait();
  outportb(0x20, 0x20);

  scancode = inportb(PORT_KBD_A);

  if (buf_top < KB_KEYS_BUFF_SIZE) {
    buffer[buf_top] = scancode;
    buf_top++;
  }

  wait();
  outportb(0x64, 0xae);		/* enable keyboard */
  wait();
}

volatile void Keyboard::led_update()
{
  u8_t cmd = 0;

  if (leds.caps)
    cmd += 4;
  if (leds.num)
    cmd += 2;
  if (leds.scroll)
    cmd += 1;

  wait();
  outportb(0x60, 0xed);
  wait();
  outportb(0x60, cmd);
}

volatile void Keyboard::wait()
{
  while ((inportb(0x64) & 2)) ;
}

size_t Keyboard::read(off_t offset, void *buf, size_t count)
{
  off_t i = 0;
  while (count) {
    while (!buf_top) ;
    buf_top--;
    ((u8_t *) buf)[i] = buffer[buf_top];
    count--;
    i++;
  }
  return (i);
}
