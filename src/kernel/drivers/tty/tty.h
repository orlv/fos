/*
  drivers/char/tty/tty.h
  Copyright (C) 2005-2008 Oleg Fedorov
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

  struct TTY_GEOMETRY {
    size_t width;
    size_t height;
  } geom;

  void scroll_up();
  void out_ch(const char ch);

  u16_t *buffer;
  u16_t *fb;

  void sync();
  
public:
   TTY(u16_t width, u16_t height);
  ~TTY();

  size_t read(off_t offset, void *buf, size_t count);
  size_t write(off_t offset, const void *buf, size_t count);

  void set_text_color(u8_t color);
  void set_bg_color(u8_t color);
};

/* Vga colors */
#define	BLACK			0
#define	BLUE			1
#define	GREEN			2
#define	CYAN			3
#define	RED			4
#define	MAGENTA			5
#define	BROWN			6
#define	LIGHTGRAY		7
#define	DARKGRAY		8
#define	LIGHTBLUE		9
#define	LIGHTGREEN		10
#define	LIGHTCYAN		11
#define	LIGHTRED		12
#define	LIGHTMAGENTA		13
#define	YELLOW			14
#define	WHITE			15

#endif
