/*
	lib/stdio.cpp
	Copyright (C) 2006 Oleg Fedorov
*/

#include <stdio.h>
#include <stdarg.h>
#include <drivers/char/tty/tty.h>
#include <vsprintf.h>
#include <system.h>
#include <hal.h>

int sprintf(char *str, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  int i = vsprintf(str, fmt, args);
  va_end(args);
  return i;
}

extern "C" int printk(const char *fmt, ...)
{
  extern TTY *stdout;
  int i = 0;
  if (stdout){
    va_list args;
    va_start(args, fmt);
    char *printbuf = new char[2000];
    i = vsprintf(printbuf, fmt, args);
    va_end(args);

    hal->mt_disable();
    stdout->write(0, printbuf, i);
    hal->mt_enable();
    delete printbuf;
  }
  return i;
}

#include <hal.h>

int tty = 0;
tid_t resolve(char *name);

int printf(const char *fmt, ...)
{
  if(!tty)
    if(!(tty = open("/dev/tty", 0)))
      return 0;

  va_list args;
  va_start(args, fmt);
  char *printbuf = new char[2048];
  size_t i = vsprintf(printbuf, fmt, args);
  va_end(args);

  i = write(tty, printbuf, i);
  delete printbuf;
  return i;
}
