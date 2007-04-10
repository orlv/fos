/*
  drivers/char/keyboard/keyboard.h
  Copyright (C) 2004-2006 Oleg Fedorov
*/

#ifndef __KEYBOARD_H
#define __KEYBOARD_H

#include <tinterface.h>
#include <types.h>

#define KB_KEYS_BUFF_SIZE 32

class Keyboard:public Tinterface {
private:

  struct leds {
    u8_t caps;
    u8_t num;
    u8_t scroll;
  } leds;

  volatile u8_t *buffer;
  volatile off_t buf_top;

  volatile void led_update();
  volatile void wait();

  volatile void SetRepeatRate(u8_t rate);

public:
   Keyboard();
  ~Keyboard();

  volatile void handler();
  size_t read(off_t offset, void *buf, size_t count);

};

#endif
