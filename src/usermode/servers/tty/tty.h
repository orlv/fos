/*
  drivers/char/tty/tty.h
  Copyright (C) 2005-2007 Oleg Fedorov
*/

#ifndef _TTY_H
#define _TTY_H

#include <types.h>

#define TTY_MODE_BLOCK  0
#define TTY_MODE_CHAR   1

class TTY {
private:
  size_t bufsize;

  u8_t textcolor;
  u8_t bgcolor;
  u16_t color;

  off_t offs;
  u32_t mode;

  struct TTY_GEOMETRY {
    size_t width;
    size_t height;
  } geom;

  void scroll_up();
  void out_ch(const char ch);
  u16_t *buffer;

public:
   TTY(u16_t width, u16_t height);
  ~TTY();

  size_t read(off_t offset, void *buf, size_t count);
  size_t write(off_t offset, const void *buf, size_t count);

  void refresh();
  void set_mode(u32_t mode);
  void set_text_color(u8_t color);
  void set_bg_color(u8_t color);

  VGA *stdout;
};

#endif
