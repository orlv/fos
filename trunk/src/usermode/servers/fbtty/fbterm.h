/*
  fbterm.h
  Copyright (C) 2007 Oleg Fedorov
*/

#ifndef _FBTERM_H
#define _FBTERM_H

#include <types.h>
#include <c++/tmutex.h>
#include "vbe.h"

class fbterm {
 private:
  u32_t _x;
  u32_t _y;

  char *fontbuf;
  u16_t *lfb;
  u16_t *lfb_cache;
  size_t lfb_size;

  vbe_mode_info_block *vbeinfo;
  u32_t scr_width;
  u32_t scr_height;

  u8_t font_width;
  u8_t font_height;
  char *font_rawdata;
  u16_t fcolor;
  u16_t bgcolor;

  size_t chars_max_cnt;
  char *chars_buf;
  size_t buf_top;

  void strip_buf();

  void sync();
  void redraw();
  void putpixel (off_t x, off_t y, u32_t pixel);
  void putpixel_direct(off_t x, off_t y, u32_t pixel);
  void put_char(off_t x, off_t y, unsigned char ch);
  void put_char_direct(off_t x, off_t y, unsigned char ch);
  void put_ch(unsigned char ch);
  void do_out_ch(unsigned char ch);
  void show_cursor(off_t x, off_t y);
  void hide_cursor(off_t x, off_t y);

  bool stop_out;

  TMutex mutex;

 public:
  fbterm();
  void out_ch(unsigned char ch);
  void bar(off_t x, off_t y, size_t x_size, size_t y_size, u16_t color);
  int load_font(char *fontpath);
  int set_videomode(u16_t mode);

  size_t write(off_t offset, const void *buf, size_t count);
};

#endif
