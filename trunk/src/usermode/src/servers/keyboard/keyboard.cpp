/*
  drivers/char/keyboard/keyboard.cpp
  Copyright (C) 2004-2007 Oleg Fedorov
*/

#include "keyboard.h"
#include <fos.h>
#include <stdio.h>
#include <io.h>

#define PORT_KBD_A      0x60

u32_t scancodes[] = {
  0,                         /* Buffer owerflow */
  0x1b, '1',   '2',  '3',  '4', /* ESC, 1, 2, 3, 4 */
  '5',  '6',  '7',  '8',  '9', /* 5, 6, 7, 8, 9 */
  '0',  '-',  '=', 0x08, 0x09, /* 0, -, =, Backspace, TAB */
  'q',  'w',  'e',  'r',  't', /* q, w, e, r, t */
  'y',  'u',  'i',  'o',  'p', /* y, u, i, o, p */
  '[',  ']',  '\n',    0,  'a', /* [, ], Enter, Ctrl, a */
  's',  'd',  'f',  'g',  'h', /* s, d, f, g, h */
  'j',  'k',  'l',  ';', 0x27, /* j, k, l, ;, ' */
  '`',    0, '\\',  'z',  'x', /* `, LShift, \, z, x */
  'c',  'v',  'b',  'n',  'm', /* c, v, b, n, m */
  ',',  '.',  '/',    0,  '*', /* ,, ., /, RShift, K* */
  0, 0x20,    0, 0x3b, 0x3c, /* Alt, Space, CapsLock, F1, F2 */
  0x3d, 0x3e, 0x3f, 0x40, 0x41, /* F3, F4, F5, F6, F7 */
  0x42, 0x43, 0x44,    0,    0, /* F8, F9, F10, NumLock, ScrollLock */
  0x47, 0x72, 0x49,  '-', 0x4b, /* Home, Up, PgUp, K-, Left */
  0, 0x77,  '+', 0x4f, 0x80, /*  , Right, K+, End, Down */
  0x51, 0x52, 0x53,    0,    0, /* PgDown, Ins, Del, SysRq, ??? */
  '\\', 0x85, 0x86,    0,    0, /* Macro, F11, F12, PA1, F13/LWin */
  0                             /* F14/RWin */
};

u32_t scancodes_shifted[] = {
  0,                   /* Buffer owerflow */
  0x1b,  '!',  '@',  '#',  '$', /* ESC, !, @, #, $ */
  '%',  '^',  '&',  '*',  '(', /* %, ^, &, *, ( */
  ')',  '_',  '+', 0x08, 0x15, /* ), _, +, Backspace, TAB */
  'Q',  'W',  'E',  'R',  'T', /* Q, W, E, R, T */
  'Y',  'U',  'I',  'O',  'P', /* Y, U, I, O, P */
  '{',  '}', 0x0d,    0,  'A', /* {, }, Enter, Ctrl, A */
  'S',  'D',  'F',  'G',  'H', /* S, D, F, G, H */
  'J',  'K',  'L',  ':',  '"', /* J, K, L, :, " */
  '~',    0,  '|',  'Z',  'X', /* ~, LShift, |, Z, X */
  'C',  'V',  'B',  'N',  'M', /* C, V, B, N, M */
  '<',  '>',  '?',    0, 0x2a, /* <, >, ?, RShift, K* */
  0, 0x20,    0, 0x54, 0x55, /* Alt, Space, CapsLock, F1, F2 */
  0x56, 0x57, 0x58, 0x59, 0x5a, /* F3, F4, F5, F6, F7 */
  0x5b, 0x5c, 0x5d,    0,    0, /* F8, F9, F10, NumLock, ScrollLock */
  0x47, 0x72, 0x49, 0x2d, 0x4b, /* Home, Up, PgUp, K-, Left */
  0, 0x77, 0x2b, 0x4f, 0x80, /*  , Right, K+, End, Down */
  0x51, 0x52, 0x53,    0,    0, /* PgDown, Ins, Del, SysRq, ??? */
  '\\', 0x87, 0x88,    0,    0, /* Macro, F11, F12, PA1, F13/LWin */
  0                          /* F14/RWin */
};

Keyboard::Keyboard()
{
  chars = new u8_t[KB_CHARS_BUFF_SIZE];

  for (int i = 0; (i8042.status_read() & i8042_OUTPUT_FULL) && i < 100; i++)
    i8042.data_read();

  i8042.wait_ready();
  i8042.command_write(i8042_SET_COMMAND);
  i8042.wait_ready();
  i8042.data_write(i8042_COMMAND);
  i8042.wait_ready();
  
  set_repeat_rate(0);
  led_update();
  code_table = scancodes;
  printf("Keyboard enabled\n");
  chars_start = 0;
  chars_top = 0;

  unmask_interrupt(KBD_IRQ_NUM);
}

Keyboard::~Keyboard()
{
  delete (u32_t *)chars;
}

void Keyboard::set_repeat_rate(u8_t rate)
{
  i8042.wait_ready();
  i8042.data_write(i8042_KBD_REPEAT_RATE);
  i8042.wait_ready();
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

  i8042.wait_ready();
  i8042.data_write(i8042_KBD_LEDS);
  i8042.wait_ready();
  i8042.data_write(cmd);
}

void Keyboard::handler()
{
  u8_t scancode;
  /*  if(key_pressed()){
    do{
    i8042.wait_ready();*/
  while(key_pressed()){
    scancode = i8042.data_read();
    decode(scancode);
  }
      /*      i8042.wait_ready();
      u8_t p = i8042.command_read();
      i8042.command_write(p | 0x80);
      i8042.command_write(p);
      decode(scancode);
    }while(key_pressed());
    }*/
  unmask_interrupt(KBD_IRQ_NUM);
}

size_t Keyboard::write(off_t offset, const void *buf, size_t count)
{
  size_t i;
  for(i=0; i<count; i++)
    if(!put(((char *)buf)[i])){
      i--;
      break;
    }

  return i;
}

size_t Keyboard::read(off_t offset, void *buf, size_t count)
{
  size_t i;
  for(i=0; i<count; i++)
    ((char *)buf)[i] = get();
    /*if(!((((char *)buf)[i]) = get())){
      i--;
      break;
      }*/

  return count;
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
