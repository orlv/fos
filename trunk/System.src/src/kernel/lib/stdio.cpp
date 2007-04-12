/*
	lib/stdio.cpp
	Copyright (C) 2006 Oleg Fedorov
*/

#include <stdio.h>
#include <stdarg.h>
#include <drivers/char/tty/tty.h>
#include <vsprintf.h>

extern TTY *stdout;

int sprintf(char *str, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  int i = vsprintf(str, fmt, args);
  va_end(args);
  return i;
}

int printf(const char *fmt, ...)
{
  /* NONE */
  return 1;
};

char printbuf[2000];

int printk(const char *fmt, ...)
{
  int i = 0;
  //string str = new char[2000];
  //if(!str) return 0;
  va_list args;
  va_start(args, fmt);
  i = vsprintf(printbuf, fmt, args);
  va_end(args);

  if (stdout)
    *stdout << printbuf;
  //stdout->write(0, str,i);
  //delete str;
  return i;
}