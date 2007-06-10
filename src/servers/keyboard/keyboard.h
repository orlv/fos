/*
  keyboard/keyboard.h
  Copyright (C) 2004-2007 Oleg Fedorov
*/

#ifndef __KEYBOARD_H
#define __KEYBOARD_H

#include <types.h>
#include <io.h>

#define KB_KEYS_BUFF_SIZE 256

class Keyboard{
 private:
  struct leds {
    u8_t caps;
    u8_t num;
    u8_t scroll;
  } leds;

  char * keys;
  size_t keys_top;
  size_t keys_start;

  void led_update();
  inline void wait() /* ожидание готовности клавиатуры */
  {
    while ((inportb(0x64) & 2));
  }

  void SetRepeatRate(u8_t rate);

 public:
  Keyboard();
  ~Keyboard();

  res_t put(char ch);
  char get();
};

#endif
