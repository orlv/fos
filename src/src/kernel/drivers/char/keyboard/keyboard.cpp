/*
  drivers/char/keyboard/keyboard.cpp
  Copyright (C) 2004-2007 Oleg Fedorov
*/

#include "keyboard.h"
#include <hal.h>
#include <system.h>
#include <stdio.h>
#include <traps.h>
#include <string.h>
#include <fs.h>

#define PORT_KBD_A      0x60

IRQ_HANDLER(keyboard_handler)
{
  extern Keyboard *keyb;
  keyb->handler();
}

inline void irq_enable_keyboard()
{
  hal->outportb(0x21, hal->inportb(0x21) & 0xfd);
}

Keyboard::Keyboard()
{
  keys = new char[KB_KEYS_BUFF_SIZE];
  data = new fs_message;
  msg = new message;
  printk("Keyboard handler installed\n");
  irq_enable_keyboard();
  keyboard_srv = 5;
}

Keyboard::~Keyboard()
{
  delete keys;
}

volatile void Keyboard::handler()
{
  char scancode;
  wait();
  hal->outportb(0x64, 0xad); /* отключим клавиатуру */
  wait();
  hal->outportb(0x20, 0x20); /* включим прерывания */

  scancode = hal->inportb(PORT_KBD_A);

  /* если буфер переполен, новые данные просто игнорируются */
  if (keys_i < KB_KEYS_BUFF_SIZE) {
    keys[keys_i] = scancode;
    keys_i++;
  }

  query();
  
  wait();
  hal->outportb(0x64, 0xae); /* включим клавиатуру */
  wait();
}

void Keyboard::query()
{
  size_t i;
  if (keys_i) {
    i = keys_i;
    keys_i = KB_KEYS_BUFF_SIZE; /* остановим добавление в
				 буфер новых данных */
    msg->send_size = sizeof(u32_t) + i;
    data->cmd = FS_CMD_WRITE;
    memcpy(data->buf, keys, i);
    msg->send_buf = data;
    msg->pid = keyboard_srv;
    send_async(msg);

    keys_i = 0;
  }
  
  return;
}
