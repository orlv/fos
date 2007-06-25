/*
  drivers/char/keyboard/keyboard.cpp
  Copyright (C) 2004-2006 Oleg Fedorov
*/

#include "keyboard.h"
#include <fos.h>
#include <stdio.h>
#include <io.h>

#define PORT_KBD_A      0x60

u32_t scancodes[] = {
  0,
  0, //ESC
  0x0031,0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x0030,
  0x002D, 0x003D, 
  8, //BACKSPACE
  '\t',//TAB
  0x0071, 0x0077, 0x0065, 0x0072, 0x0074, 0x0079, 0x0075, 0x0069, 0x006F, 0x0070, 0x005B, 0x005D,
  '\n', //ENTER
  0, //CTRL
  0x0061, 0x0073, 0x0064, 0x0066, 0x0067, 0x0068, 0x006A, 0x006B, 0x006C, 0x003B, 0x0027, 0x0060,
  0, //LEFT SHIFT,
  0x005C, 0x007A, 0x0078, 0x0063, 0x0076, 0x0062, 0x006E, 0x005D, 0x002C, 0x002E, 0x002F,
  0, //RIGHT SHIFT,
  0x002A, //NUMPAD
  0, //ALT
  0x0020, //SPACE
  0, //CAPSLOCK
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //F1 - F10
  0, //NUMLOCK
  0, //SCROLLOCK
  0, //HOME
  0, 
  0, //PAGE UP
  0x002D, //NUMPAD
  0, 0,
  0, //(r)
  0x002B, //NUMPAD
  0, //END
  0, 
  0, //PAGE DOWN
  0, //INS
  0, //DEL
  0, //SYS RQ
  0, 
  0, 0, //F11-F12
  0,
  0, 0, 0, //F13-F15
  0, 0, 0, 0, 0, 0, 0, 0, 0, //F16-F24
  0, 0, 0, 0, 0, 0, 0, 0
};

u32_t scancodes_shifted[] = {
  0,
  0, //ESC
  0x0021, 0x0040, 0x0023, 0x0024, 0x0025, 0x005E, 0x0026, 0x002A, 0x0028, 0x0029,
  0x005F, 0x002B, 
  8, //BACKSPACE
  '\t',//TAB
  0x0051, 0x0057, 0x0045, 0x0052, 0x0054, 0x0059, 0x0055, 0x0049, 0x004F, 0x0050, 0x007B, 0x007D,
  '\n', //ENTER
  0, //CTRL
  0x0041, 0x0053, 0x0044, 0x0046, 0x0047, 0x0048, 0x004A, 0x004B, 0x004C, 0x003A, 0x0022, 0x007E,
  0, //LEFT SHIFT,
  0x007C, 0x005A, 0x0058, 0x0043, 0x0056, 0x0042, 0x004E, 0x004D, 0x003C, 0x002E, 0x003F,
  0, //RIGHT SHIFT,
  0x002A, //NUMPAD
  0, //ALT
  0x0020, //SPACE
  0, //CAPSLOCK
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //F1 - F10
  0, //NUMLOCK
  0, //SCROLLOCK
  0, //HOME
  0, 
  0, //PAGE UP
  0x002D, //NUMPAD
  0, 0,
  0, //(r)
  0x002B, //NUMPAD
  0, //END
  0, 
  0, //PAGE DOWN
  0, //INS
  0, //DEL
  0, //SYS RQ
  0, 
  0, 0, //F11-F12
  0,
  0, 0, 0, //F13-F15
  0, 0, 0, 0, 0, 0, 0, 0, 0, //F16-F24
  0, 0, 0, 0, 0, 0, 0, 0
};

Keyboard::Keyboard()
{
  chars = new u8_t[KB_CHARS_BUFF_SIZE];
  unmask_interrupt(KBD_IRQ_NUM);
  set_repeat_rate(0);
  led_update();
  code_table = scancodes;
  printf("Keyboard enabled\n");
  chars_start = 0;
  chars_top = 0;
}

Keyboard::~Keyboard()
{
  delete (u32_t *)chars;
}

void Keyboard::set_repeat_rate(u8_t rate)
{
  wait_ready();
  i8042.data_write(i8042_KBD_REPEAT_RATE);
  wait_ready();
  i8042.data_write(rate);
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

  wait_ready();
  i8042.data_write(i8042_KBD_LEDS);
  wait_ready();
  i8042.data_write(cmd);
}

void Keyboard::handler()
{
  u8_t scancode = i8042.data_read();
  
  u8_t p = i8042.command_read();
  i8042.command_write(p | 0x80);
  i8042.command_write(p);
  
  unmask_interrupt(KBD_IRQ_NUM);
  decode(scancode);
}

res_t Keyboard::put(u8_t ch)
{
  buffer_mutex.lock();
  size_t i = chars_top + 1;
  res_t res = RES_FAULT;
  
  if(i == KB_CHARS_BUFF_SIZE)
    i = 0;

  if(i != chars_start){
    chars[chars_top] = ch;
    chars_top = i;
    res = RES_SUCCESS;
  }

  buffer_mutex.unlock();
  return res;
}

u8_t Keyboard::get()
{
  u8_t ch;

  while(1){
    while(chars_start == chars_top);
    
    buffer_mutex.lock();
    if(chars_start != chars_top){
      ch = chars[chars_start];
      chars_start++;
      if(chars_start == KB_CHARS_BUFF_SIZE)
	chars_start = 0;
      buffer_mutex.unlock();
      break;
    }
    buffer_mutex.unlock();
  }

  return ch;
}

void Keyboard::decode(u8_t scancode)
{
  bool released;
  if (scancode == 0xe0) {
    extended = 1;
  } else {
    released = scancode & 0x80;
    scancode &= 0x7f;
    
    if (extended) {
      scancode |= 0x80;
      extended = 0;
    }

    if (released) {
      switch (scancode) {
      case 0x36:
      case 0x2A:
	keys.shift = 0;
	code_table = scancodes;
	break;
	
      case 0x38:
	keys.alt = 0;
	break;
	
      case 0x1D:
	keys.ctrl = 0;
	break;
      }
    } else {
      switch (scancode) {
      case 0x36: 
      case 0x2A:
	keys.shift = 1;
	code_table = scancodes_shifted;
	break;
	
      case 0x38:
	keys.alt = 1;
	break;
	
      case 0x1D:
	keys.ctrl = 1;
	break;
	
      default:
	if (code_table[scancode] != 0){
	  put(code_table[scancode]);
	}
	break;
      }
    }
  }
}
