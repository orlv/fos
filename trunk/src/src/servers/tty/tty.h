/*
  drivers/char/tty/tty.h
  Copyright (C) 2005-2006 Oleg Fedorov
*/

#ifndef _TTY_H
#define _TTY_H

#include <types.h>
//#include <tinterface.h>

class TTY			/*: public Tinterface */
{
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
  void OutRaw(u8_t ch);
  void Out(const char ch);
  void outs(const char *str, size_t len);
  p_u16_t buffer;

public:
   TTY(u16_t width, u16_t height);
  ~TTY();

  //size_t read(off_t offset, void *buf, size_t count);
  size_t write(off_t offset, const void *buf, size_t count);

  void outs(const char *str);
  void refresh();

  void SetTextColor(u8_t tcolor);
  void SetBgColor(u8_t tcolor);

  VGA *stdout;
};

#endif
