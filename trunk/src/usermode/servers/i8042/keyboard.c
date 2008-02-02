#include <fos/fos.h>
#include <sys/io.h>
#include <stdio.h>
#include <stdlib.h>
#include "keyboard.h"
#include "i8042.h"

#define debug_printf printf

u8_t *volatile chars;
size_t volatile chars_top;
size_t volatile chars_start;

u32_t scancodes[] = {
  0,				/* Buffer owerflow */
  0x1b, '1', '2', '3', '4',	/* ESC, 1, 2, 3, 4 */
  '5', '6', '7', '8', '9',	/* 5, 6, 7, 8, 9 */
  '0', '-', '=', 0x08, 0x09,	/* 0, -, =, Backspace, TAB */
  'q', 'w', 'e', 'r', 't',	/* q, w, e, r, t */
  'y', 'u', 'i', 'o', 'p',	/* y, u, i, o, p */
  '[', ']', '\n', 0, 'a',	/* [, ], Enter, Ctrl, a */
  's', 'd', 'f', 'g', 'h',	/* s, d, f, g, h */
  'j', 'k', 'l', ';', 0x27,	/* j, k, l, ;, ' */
  '`', 0, '\\', 'z', 'x',	/* `, LShift, \, z, x */
  'c', 'v', 'b', 'n', 'm',	/* c, v, b, n, m */
  ',', '.', '/', 0, '*',	/* ,, ., /, RShift, K* */
  0, 0x20, 0, 0xF03b, 0xF03c,	/* Alt, Space, CapsLock, F1, F2 */
  0xF03d, 0xF03e, 0xF03f, 0xF040, 0xF041,	/* F3, F4, F5, F6, F7 */
  0xF042, 0xF043, 0xF044, 0, 0,	/* F8, F9, F10, NumLock, ScrollLock */
  0xF047, 0xF072, 0xF049, '-', 0xF04b,	/* Home, Up, PgUp, K-, Left */
  0, 0xF077, '+', 0xF04f, 0xF080,	/*  , Right, K+, End, Down */
  0xF051, 0xF052, 0xF053, 0, 0,	/* PgDown, Ins, Del, SysRq, ??? */
  '\\', 0xF085, 0xF086, 0, 0,	/* Macro, F11, F12, PA1, F13/LWin */
  0,
  0xFE00, 0xFE01, 0xFE02, 0xFE03, 0xFE04, 0xFE05, 0xFE06, 0xFE07,
  0xFE08, 0xFE09, 0xFE0a, 0xFE0b, 0xFE0c, 0xFE0d, 0xFE0e, 0xFE0f,
  0xFE10, 0xFE11, 0xFE12, 0xFE13, 0xFE14, 0xFE15, 0xFE16, 0xFE17,
  0xFE18, 0xFE19, 0xFE1a, 0xFE1b, 0xFE1c, 0xFE1d, 0xFE1e, 0xFE1f,
  0xFE20, 0xFE21, 0xFE22, 0xFE23, 0xFE24, 0xFE25, 0xFE26, 0xFE27,
  0xFE28, 0xFE29, 0xFE2a, 0xFE2b, 0xFE2c, 0xFE2d, 0xFE2e, 0xFE2f,
  0xFE30, 0xFE31, 0xFE32, 0xFE33, 0xFE34, 0xFE35, 0xFE36, 0xFE37,
  0xFE38, 0xFE39, 0xFE3a, 0xFE3b, 0xFE3c, 0xFE3d, 0xFE3e, 0xFE3f,
  0xFE40, 0xFE41, 0xFE42, 0xFE43, 0xFE44, 0xFE45, 0xFE46, 0xFE47,
  0xFE48, 0xFE49, 0xFE4a, 0xFE4b, 0xFE4c, 0xFE4d, 0xFE4e, 0xFE4f,
  0xFE50, 0xFE51, 0xFE52, 0xFE53, 0xFE54, 0xFE55, 0xFE56, 0xFE57,
  0xFE58, 0xFE59, 0xFE5a, 0xFE5b, 0xFE5c, 0xFE5d, 0xFE5e, 0xFE5f,
  0xFE60, 0xFE61, 0xFE62, 0xFE63, 0xFE64, 0xFE65, 0xFE66, 0xFE67,
  0xFE68, 0xFE69, 0xFE6a, 0xFE6b, 0xF072, 0xFE6d, 0xFE6e, 0xF04B,
  0xFE70, 0xF077, 0xFE72, 0xFE73, 0xF080, 0xFE75, 0xFE76, 0xFE77,
  0xFE78, 0xFE79, 0xFE7a, 0xFE7b, 0xFE7c, 0xFE7d, 0xFE7e, 0xFE7f,
  0xFE80, 0xFE81, 0xFE82, 0xFE83, 0xFE84, 0xFE85, 0xFE86, 0xFE87,
  0xFE88, 0xFE89, 0xFE8a, 0xFE8b, 0xFE8c, 0xFE8d, 0xFE8e, 0xFE8f,
  0xFE90, 0xFE91, 0xFE92, 0xFE93, 0xFE94, 0xFE95, 0xFE96, 0xFE97,
  0xFE98, 0xFE99, 0xFE9a, 0xFE9b, 0xFE9c, 0xFE9d, 0xFE9e, 0xFE9f,
  0xFEa0, 0xFEa1, 0xFEa2, 0xFEa3,	/* F14/RWin */
};

u32_t scancodes_shifted[] = {
  0,				/* Buffer owerflow */
  0x1b, '!', '@', '#', '$',	/* ESC, !, @, #, $ */
  '%', '^', '&', '*', '(',	/* %, ^, &, *, ( */
  ')', '_', '+', 0x08, 0x15,	/* ), _, +, Backspace, TAB */
  'Q', 'W', 'E', 'R', 'T',	/* Q, W, E, R, T */
  'Y', 'U', 'I', 'O', 'P',	/* Y, U, I, O, P */
  '{', '}', 0x0d, 0, 'A',	/* {, }, Enter, Ctrl, A */
  'S', 'D', 'F', 'G', 'H',	/* S, D, F, G, H */
  'J', 'K', 'L', ':', '"', /* J, K, L, :, " */ '~', 0, '|', 'Z', 'X',	/* ~, LShift, |, Z, X */
  'C', 'V', 'B', 'N', 'M',	/* C, V, B, N, M */
  '<', '>', '?', 0, 0x2a,	/* <, >, ?, RShift, K* */
  0, 0x20, 0, 0xF054, 0xF055,	/* Alt, Space, CapsLock, F1, F2 */
  0xF056, 0xF057, 0xF058, 0xF059, 0xF05a,	/* F3, F4, F5, F6, F7 */
  0xF05b, 0xF05c, 0xF05d, 0, 0,	/* F8, F9, F10, NumLock, ScrollLock */
  0xF047, 0xF072, 0xF049, 0x2d, 0xF04b,	/* Home, Up, PgUp, K-, Left */
  0, 0xF077, 0x2b, 0xF04f, 0xF080,	/*  , Right, K+, End, Down */
  0xF051, 0xF052, 0xF053, 0, 0,	/* PgDown, Ins, Del, SysRq, ??? */
  '\\', 0xF087, 0xF088, 0, 0,	/* Macro, F11, F12, PA1, F13/LWin */
  0,
  0xFE00, 0xFE01, 0xFE02, 0xFE03, 0xFE04, 0xFE05, 0xFE06, 0xFE07,
  0xFE08, 0xFE09, 0xFE0a, 0xFE0b, 0xFE0c, 0xFE0d, 0xFE0e, 0xFE0f,
  0xFE10, 0xFE11, 0xFE12, 0xFE13, 0xFE14, 0xFE15, 0xFE16, 0xFE17,
  0xFE18, 0xFE19, 0xFE1a, 0xFE1b, 0xFE1c, 0xFE1d, 0xFE1e, 0xFE1f,
  0xFE20, 0xFE21, 0xFE22, 0xFE23, 0xFE24, 0xFE25, 0xFE26, 0xFE27,
  0xFE28, 0xFE29, 0xFE2a, 0xFE2b, 0xFE2c, 0xFE2d, 0xFE2e, 0xFE2f,
  0xFE30, 0xFE31, 0xFE32, 0xFE33, 0xFE34, 0xFE35, 0xFE36, 0xFE37,
  0xFE38, 0xFE39, 0xFE3a, 0xFE3b, 0xFE3c, 0xFE3d, 0xFE3e, 0xFE3f,
  0xFE40, 0xFE41, 0xFE42, 0xFE43, 0xFE44, 0xFE45, 0xFE46, 0xFE47,
  0xFE48, 0xFE49, 0xFE4a, 0xFE4b, 0xFE4c, 0xFE4d, 0xFE4e, 0xFE4f,
  0xFE50, 0xFE51, 0xFE52, 0xFE53, 0xFE54, 0xFE55, 0xFE56, 0xFE57,
  0xFE58, 0xFE59, 0xFE5a, 0xFE5b, 0xFE5c, 0xFE5d, 0xFE5e, 0xFE5f,
  0xFE60, 0xFE61, 0xFE62, 0xFE63, 0xFE64, 0xFE65, 0xFE66, 0xFE67,
  0xFE68, 0xFE69, 0xFE6a, 0xFE6b, 0xF072, 0xFE6d, 0xFE6e, 0xF04B,
  0xFE70, 0xF077, 0xFE72, 0xFE73, 0xF080, 0xFE75, 0xFE76, 0xFE77,
  0xFE78, 0xFE79, 0xFE7a, 0xFE7b, 0xFE7c, 0xFE7d, 0xFE7e, 0xFE7f,
  0xFE80, 0xFE81, 0xFE82, 0xFE83, 0xFE84, 0xFE85, 0xFE86, 0xFE87,
  0xFE88, 0xFE89, 0xFE8a, 0xFE8b, 0xFE8c, 0xFE8d, 0xFE8e, 0xFE8f,
  0xFE90, 0xFE91, 0xFE92, 0xFE93, 0xFE94, 0xFE95, 0xFE96, 0xFE97,
  0xFE98, 0xFE99, 0xFE9a, 0xFE9b, 0xFE9c, 0xFE9d, 0xFE9e, 0xFE9f,
  0xFEa0, 0xFEa1, 0xFEa2, 0xFEa3,	/* F14/RWin */
};

#undef CONSTRUCTION_KEYCODES

u32_t *code_table = scancodes;
int extended = 0;

struct keys {
  int shift;
  int ctrl;
  int alt;
} keys;

res_t keyboard_put(u8_t ch);

static int set_rate_delay(int cps, int msec)
{
  unsigned char param;

  if (cps < 3)
    param = 0x1f;		/* 2 chars/sec */
  else if (cps < 4)
    param = 0x1a;		/* 3 chars/sec */
  else if (cps < 5)
    param = 0x17;		/* 4 chars/sec */
  else if (cps < 6)
    param = 0x14;		/* 5 chars/sec */
  else if (cps < 7)
    param = 0x12;		/* 6 chars/sec */
  else if (cps < 10)
    param = 0x0f;		/* 8 chars/sec */
  else if (cps < 11)
    param = 0x0c;		/* 10 chars/sec */
  else if (cps < 13)
    param = 0x0a;		/* 12 chars/sec */
  else if (cps < 16)
    param = 0x08;		/* 15 chars/sec */
  else if (cps < 20)
    param = 0x06;		/* 17 chars/sec */
  else if (cps < 23)
    param = 0x04;		/* 21 chars/sec */
  else if (cps < 26)
    param = 0x02;		/* 24 chars/sec */
  else if (cps < 29)
    param = 0x01;		/* 27 chars/sec */
  else
    param = 0x00;		/* 30 chars/sec */

  if (msec > 800)
    param |= 0x60;		/* 1.0 sec */
  else if (msec > 600)
    param |= 0x40;		/* 0.75 sec */
  else if (msec > 400)
    param |= 0x20;		/* 0.5 sec */

  return i8042_kbd_command(KBDK_TYPEMATIC, param);
}

static void receive_byte(unsigned char scancode)
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
	if (code_table[scancode] != 0) {
	  if ((code_table[scancode] & 0xFE00) == 0xFE00) {	// несделанный сканкод
#ifdef CONSTRUCTION_KEYCODES
	    printf("Construction scancode: %x\n", code_table[scancode]);
#endif
	    break;
	  }
	  if (code_table[scancode] & 0xF000)	// расширенный сканкод
	    kb_put(0x01);
	  kb_put(code_table[scancode] & 0xFF);
	}
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
void keyboard_ps2_interrupt()
{
  unsigned char c, strobe, sts;

  /* Read the pending information. */
  sts = inb(KBDC_AT_CTL);
  c = inb(KBD_DATA);

  strobe = inb(KBDC_XT_CTL);
  outb(strobe | KBDC_XT_CLEAR, KBDC_XT_CTL);
  outb(strobe, KBDC_XT_CTL);
  receive_byte(c);
  unmask_interrupt(1);
}

res_t kb_put(u8_t ch)
{
  size_t i = chars_top + 1;
  res_t res = RES_FAULT;

  if (i == KB_CHARS_BUFF_SIZE)
    i = 0;

  if (i != chars_start) {
    chars[chars_top] = ch;
    chars_top = i;
    res = RES_SUCCESS;
  }

  return res;
}

static u8_t kb_get()
{
  char ch;

  while (1) {
    while (chars_start == chars_top) ;

    if (chars_start != chars_top) {
      ch = chars[chars_start];
      chars_start++;
      if (chars_start == KB_CHARS_BUFF_SIZE)
	chars_start = 0;
      break;
    }
  }
  return ch;
}

size_t kb_write(off_t offset, const void *buf, size_t count)
{
  size_t i;

  for (i = 0; i < count; i++)
    if (!kb_put(((char *)buf)[i])) {
      i--;
      break;
    }

  return i;
}

size_t kb_read(off_t offset, void *buf, size_t count)
{
  size_t i;

  for (i = 0; i < count; i++)
    ((char *)buf)[i] = kb_get();
  /*if(!((((char *)buf)[i]) = get())){
     i--;
     break;
     } */

  return count;
}

void keyboard_ps2_init()
{

//      lock_take_irq (&u->lock, RECEIVE_IRQ,
//              (handler_t) keyboard_ps2_interrupt, u);

  i8042_kbd_enable();
  if (!i8042_kbd_probe())
    return;
  set_rate_delay(20, 500);
  chars = malloc(KB_CHARS_BUFF_SIZE);
}
