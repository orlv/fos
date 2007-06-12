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

#if 0
unsigned char scan2ascii[]={    0,                         /* Buffer owerflow */
                       0x1b, '1',   '2',  '3',  '4', /* ESC, 1, 2, 3, 4 */
                       '5',  '6',  '7',  '8',  '9', /* 5, 6, 7, 8, 9 */
                       '0',  '-',  '=', 0x08, 0x09, /* 0, -, =, Backspace, TAB */
                       'q',  'w',  'e',  'r',  't', /* q, w, e, r, t */
                       'y',  'u',  'i',  'o',  'p', /* y, u, i, o, p */
                       '[',  ']', 0x0d,    0,  'a', /* [, ], Enter, Ctrl, a */
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

unsigned char scan2ascii_shift[]={    0,                   /* Buffer owerflow */
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

unsigned char scan2ascii_ctrl[]={    0,                         /* Buffer owerflow */
                         0x1b,    0, 0x03,    0,    0, /* ESC, 1, 2, 3, 4 */
                            0, 0x1e,    0,    0,    0, /* 5, 6, 7, 8, 9 */
                            0, 0x1f,    0,    0, 0x94, /* 0, -, =, Backspace, TAB */
                         0x11, 0x17, 0x05, 0x12, 0x14, /* q, w, e, r, t */
                         0x19, 0x15, 0x09, 0x0f, 0x10, /* y, u, i, o, p */
                         0x1b, 0x1d, 0x0a,    0, 0x01, /* [, ], Enter, Ctrl, a */
                         0x13, 0x04, 0x06, 0x07, 0x08, /* s, d, f, g, h */
                         0x0a, 0x0b, 0x0c,    0,    0, /* j, k, l, ;, ' */
                            0,    0, 0x1c, 0x1a, 0x18, /* `, LShift, \, z, x */
                         0x03, 0x16, 0x02, 0x0e, 0x0d, /* c, v, b, n, m */
                            0,    0, 0x7f,    0, 0x96, /* ,, ., /, RShift, K* */
                            0, 0x20,    0, 0x5e, 0x5f, /* Alt, Space, CapsLock, F1, F2 */
                         0x60, 0x61, 0x62, 0x63, 0x64, /* F3, F4, F5, F6, F7 */
                         0x65, 0x66, 0x67,    0,    0, /* F8, F9, F10, NumLock, ScrollLock */
                         0x77, 0x8d, 0x84, 0x8e, 0x73, /* Home, Up, PgUp, K-, Left */
                            0, 0x74, 0x90, 0x75, 0x91, /*  , Right, K+, End, Down */
                         0x76, 0x92, 0x93, 0x10,    0, /* PgDown, Ins, Del, SysRq, ??? */
                            0, 0x89, 0x8a,    0,    0, /* Macro, F11, F12, PA1, F13/LWin */
                            0                          /* F14/RWin */
                       };

unsigned char scan2ascii_alt[]={    0,                         /* Buffer owerflow */
                        0x01, 0x78, 0x79, 0x7a, 0x7b, /* ESC, 1, 2, 3, 4 */
                        0x7c, 0x7d, 0x7e, 0x7f, 0x80, /* 5, 6, 7, 8, 9 */
                        0x81, 0x82, 0x83, 0x0e, 0xa5, /* 0, -, =, Backspace, TAB */
                        0x10, 0x11, 0x12, 0x13, 0x14, /* q, w, e, r, t */
                        0x15, 0x16, 0x17, 0x18, 0x19, /* y, u, i, o, p */
                        0x1a, 0x1b, 0x1c,    0, 0x1e, /* [, ], Enter, Ctrl, a */
                        0x1f, 0x20, 0x21, 0x22, 0x23, /* s, d, f, g, h */
                        0x24, 0x25, 0x26, 0x27, 0x28, /* j, k, l, ;, ' */
                        0x29,    0, 0x2b, 0x2c, 0x2d, /* `, LShift, \, z, x */
                        0x2e, 0x2f, 0x30, 0x31, 0x32, /* c, v, b, n, m */
                        0x33, 0x34, 0x35,    0, 0x37, /* ,, ., /, RShift, K* */
                           0, 0x20,    0, 0x68, 0x69, /* Alt, Space, CapsLock, F1, F2 */
                        0x6a, 0x6b, 0x6c, 0x6d, 0x6e, /* F3, F4, F5, F6, F7 */
                        0x6f, 0x70, 0x71,    0,    0, /* F8, F9, F10, NumLock, ScrollLock */
                        0x97, 0x98, 0x99, 0x4a, 0x9b, /* Home, Up, PgUp, K-, Left */
                           0, 0x9d, 0x4e, 0x9f, 0xa0, /*  , Right, K+, End, Down */
                        0xa1, 0xa2, 0xa3,    0,    0, /* PgDown, Ins, Del, SysRq, ??? */
                           0, 0x8b, 0x8c,    0,    0, /* Macro, F11, F12, PA1, F13/LWin */
                           0                          /* F14/RWin */
                     };

unsigned char need_zero[]={  0,             /* Buffer owerflow */
                    1, 1, 1, 1, 1, /* ESC, 1, 2, 3, 4 */
                    1, 1, 1, 1, 1, /* 5, 6, 7, 8, 9 */
                    1, 1, 1, 1, 1, /* ), _, +, Backspace, TAB */
                    1, 1, 1, 1, 1, /* q, w, e, r, t */
                    1, 1, 1, 1, 1, /* y, u, i, o, p */
                    1, 1, 1, 1, 1, /* [, ], Enter, Ctrl, a */
                    1, 1, 1, 1, 1, /* s, d, f, g, h */
                    1, 1, 1, 1, 1, /* j, k, l, ;, ' */
                    1, 1, 1, 1, 1, /* `, LShift, \, z, x */
                    1, 1, 1, 1, 1, /* c, v, b, n, m */
                    1, 1, 1, 1, 1, /* ,, ., /, RShift, K* */
                    1, 1, 1, 0, 0, /* Alt, Space, CapsLock, F1, F2 */
                    0, 0, 0, 0, 0, /* F3, F4, F5, F6, F7 */
                    0, 0, 0, 1, 1, /* F8, F9, F10, NumLock, ScrollLock */
                    0, 0, 0, 1, 0, /* Home, Up, PgUp, K-, Left */
                    1, 0, 1, 0, 0, /*  , Right, K+, End, Down */
                    0, 0, 0, 1, 1, /* PgDown, Ins, Del, SysRq, ??? */
                    1, 0, 0, 1, 1, /* Macro, F11, F12, PA1, F13/LWin */
                    1              /* F14/RWin */
                };

#endif

#define KEYBOARD_IRQ_NUM 1
#define PORT_KBD_A       0x60

class Keyboard{
 private:
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

  volatile char * chars;
  volatile size_t chars_top;
  volatile size_t chars_start;
  TMutex buffer_mutex;
  
  void led_update();
  inline void wait() /* ожидание готовности клавиатуры */
  {
    while ((inportb(0x64) & 2));
  }

  void set_repeat_rate(u8_t rate);
  void decode(u8_t scancode);

 public:
  Keyboard();
  ~Keyboard();

  res_t put(char ch);
  char get();
  void handler();
};

#endif
