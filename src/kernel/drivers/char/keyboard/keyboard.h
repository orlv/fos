/*
  drivers/char/keyboard/keyboard.h
  Copyright (C) 2004-2007 Oleg Fedorov
*/

#ifndef __KEYBOARD_H
#define __KEYBOARD_H

#include <types.h>
#include <hal.h>

#define KB_KEYS_BUFF_SIZE 256

class Keyboard{
private:
  char * keys;
  size_t keys_i;
  sid_t keyboard_srv;
  struct fs_message *data;
  struct message *msg;
  

  inline void wait() /* ожидание готовности клавиатуры */
  {
    while ((hal->inportb(0x64) & 2));
  }

public:
  Keyboard();
  ~Keyboard();

  volatile void handler();
  void query();
};


#endif
