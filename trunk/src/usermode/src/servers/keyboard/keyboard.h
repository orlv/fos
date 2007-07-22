/*
  keyboard/keyboard.h
  Copyright (C) 2004-2007 Oleg Fedorov
  
*/

#ifndef __KEYBOARD_H
#define __KEYBOARD_H

#include <types.h>
#include <io.h>
#include <tmutex.h>

#define KB_CHARS_BUFF_SIZE 256

#define KBD_IRQ_NUM            1

#define i8042_DATA             0x60
#define i8042_CMD              0x61
#define i8042_STATUS           0x64

#define i8042_OUTPUT_FULL      0x01
#define i8042_INPUT_FULL       0x02

#define i8042_SET_COMMAND      0x60
#define i8042_COMMAND          0x49

/* Команды клавиатуры */
#define i8042_KBD_LEDS         0xed /* изменить состояние светодиодов клавиатуры */
#define i8042_KBD_ECHO         0xee /* эхо-запрос, клавиатура отвечает скан-кодом 0xee */
#define i8042_KBD_REPEAT_RATE  0xf3 /* установить параметры режима автоповтора */
#define i8042_KBD_ENABLE       0xf4 /* включить клавиатуру */
#define i8042_KBD_DISABLE      0xf5 /* выключить клавиатуру */
#define i8042_KBD_DEFAULT      0xf6 /* установить параметры по умолчанию */
#define i8042_KBD_RETRY        0xfe /* послать скан-код ещё раз */

class i8042 {
 public:
  inline u8_t data_read()
  {
    return inb(i8042_DATA);
  }

  inline void data_write(u8_t data)
  {
    outb(data, i8042_DATA);
  }
  
  inline u8_t status_read()
  {
    return inb(i8042_STATUS);
  }

  inline void status_write(u8_t data)
  {
    outb(data, i8042_STATUS);
  }
  
  inline u8_t command_read()
  {
    return inb(i8042_CMD);
  }

  inline void command_write(u8_t cmd)
  {
    outb(cmd, i8042_CMD);
  }

  inline void wait_ready()
  {
    while ((status_read() & i8042_INPUT_FULL));
  }
};

class Keyboard{
 private:
  class i8042 i8042;
  
  struct leds {
    bool caps;
    bool num;
    bool scroll;
  } leds;

  struct keys {
    bool shift;
    bool ctrl;
    bool alt;
  } keys;

  bool extended;
  u32_t * code_table;
  u8_t kbd_read_mask;
  u8_t * volatile chars;
  size_t volatile chars_top;
  size_t volatile chars_start;
  TMutex buffer_mutex;
  
  void led_update();

  inline bool key_pressed()
  {
    return (i8042.status_read() & i8042_OUTPUT_FULL) == 1;
  }
  
  void set_repeat_rate(u8_t rate);
  void decode(u8_t scancode);
  
 public:
  Keyboard();
  ~Keyboard();

  res_t put(u8_t ch);
  u8_t get();
  void handler();

  size_t write(off_t offset, const void *buf, size_t count);
  size_t read(off_t offset, void *buf, size_t count);
};

#endif
