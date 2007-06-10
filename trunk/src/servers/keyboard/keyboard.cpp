/*
  drivers/char/keyboard/keyboard.cpp
  Copyright (C) 2004-2006 Oleg Fedorov
*/

#include "keyboard.h"
#include <fos.h>
#include <stdio.h>
#include <io.h>

#define PORT_KBD_A      0x60

inline void irq_enable_keyboard()
{
  outportb(0x21, inportb(0x21) & 0xfd);
}

Keyboard::Keyboard()
{
  keys = new char[KB_KEYS_BUFF_SIZE];
  //irq_enable_keyboard();
  //SetRepeatRate(0);
  //led_update();
  printf("Keyboard enabled\n");
}

Keyboard::~Keyboard()
{
  delete (char *)keys;
}

void Keyboard::SetRepeatRate(u8_t rate)
{
  wait();
  outportb(0x60, 0xf3);
  wait();
  outportb(0x60, rate);
}

void Keyboard::led_update()
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


res_t Keyboard::put(char ch)
{
  size_t i = keys_top + 1;
  if(i == KB_KEYS_BUFF_SIZE)
    i = 0;

  if(i != keys_start){
    keys[keys_top] = ch;
    keys_top = i;
    return RES_SUCCESS;
  }

  return RES_FAULT;
}


char Keyboard::get()
{
  char ch;

  if(keys_start != keys_top){
    ch = keys[keys_start];
    keys_start++;
    if(keys_start == KB_KEYS_BUFF_SIZE)
      keys_start = 0;
  } else {
    ch = 0;
  }
    
  return ch;
}
