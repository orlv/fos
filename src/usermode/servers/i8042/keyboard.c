#include <fos/fos.h>
#include <sys/io.h>
#include <stdio.h>
#include <stdlib.h>
#include "keyboard.h"
#include "i8042.h"
#define debug_printf printf
  u8_t * volatile chars;
  size_t volatile chars_top;
  size_t volatile chars_start;
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

u32_t * code_table = &scancodes;
int extended = 0;
  struct keys {
    int shift;
    int ctrl;
    int alt;
  } keys;
res_t keyboard_put(u8_t ch);

static int
set_rate_delay (int cps, int msec)
{
	unsigned char param;

	if      (cps < 3)  param = 0x1f;	/* 2 chars/sec */
	else if (cps < 4)  param = 0x1a;	/* 3 chars/sec */
	else if (cps < 5)  param = 0x17;	/* 4 chars/sec */
	else if (cps < 6)  param = 0x14;	/* 5 chars/sec */
	else if (cps < 7)  param = 0x12;	/* 6 chars/sec */
	else if (cps < 10) param = 0x0f;	/* 8 chars/sec */
	else if (cps < 11) param = 0x0c;	/* 10 chars/sec */
	else if (cps < 13) param = 0x0a;	/* 12 chars/sec */
	else if (cps < 16) param = 0x08;	/* 15 chars/sec */
	else if (cps < 20) param = 0x06;	/* 17 chars/sec */
	else if (cps < 23) param = 0x04;	/* 21 chars/sec */
	else if (cps < 26) param = 0x02;	/* 24 chars/sec */
	else if (cps < 29) param = 0x01;	/* 27 chars/sec */
	else               param = 0x00;	/* 30 chars/sec */

	if      (msec > 800) param |= 0x60;	/* 1.0 sec */
	else if (msec > 600) param |= 0x40;	/* 0.75 sec */
	else if (msec > 400) param |= 0x20;	/* 0.5 sec */

	return i8042_kbd_command (KBDK_TYPEMATIC, param);
}

static void receive_byte (unsigned char scancode)
{
  int released;
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
	if (code_table[scancode] != 0)
	  kb_put(code_table[scancode]);

	break;
      }
    }
  }
}

/*
 * Process keyboard interrupt.
 * Get raw data and keyboard_put to buffer. Return 1 when not enough data
 * for a event record.
 * Return 0 when a new event record is generated and
 * a signal to keyboard task is needed.
 */
void keyboard_ps2_interrupt ()
{
	unsigned char c, strobe, sts;

	/* Read the pending information. */
	sts = inb (KBDC_AT_CTL);
	c = inb (KBD_DATA);

	strobe = inb (KBDC_XT_CTL);
	outb (strobe | KBDC_XT_CLEAR, KBDC_XT_CTL);
	outb (strobe, KBDC_XT_CTL);
	receive_byte (c);
	unmask_interrupt(1);
}
res_t kb_put(u8_t ch)
{
  size_t i = chars_top + 1;
  res_t res = RES_FAULT;
  
  if(i == KB_CHARS_BUFF_SIZE)
    i = 0;

  if(i != chars_start){
    chars[chars_top] = ch;
    chars_top = i;
    res = RES_SUCCESS;
  }

  return res;
}

static u8_t kb_get()
{
	char ch;
 while(1){
    while(chars_start == chars_top);
    
    if(chars_start != chars_top){
      ch = chars[chars_start];
      chars_start++;
      if(chars_start == KB_CHARS_BUFF_SIZE)
	chars_start = 0;
      break;
    }
}
	return ch;
}
size_t kb_write(off_t offset, const void *buf, size_t count)
{
  size_t i;
  for(i=0; i<count; i++)
    if(!kb_put(((char *)buf)[i])){
      i--;
      break;
    }

  return i;
}
size_t kb_read(off_t offset, void *buf, size_t count)
{
  size_t i;
  for(i=0; i<count; i++)
    ((char *)buf)[i] = kb_get();
    /*if(!((((char *)buf)[i]) = get())){
      i--;
      break;
      }*/

  return count;
}

void
keyboard_ps2_init ()
{

//	lock_take_irq (&u->lock, RECEIVE_IRQ,
//		(handler_t) keyboard_ps2_interrupt, u);

	i8042_kbd_enable ();
	if (!i8042_kbd_probe ())
		return;
	set_rate_delay (20, 500);
	chars = malloc(KB_CHARS_BUFF_SIZE);
}
