/*
    lib/stdio.c
    Copyright (C) 2006-2007 Oleg Fedorov
*/

#include <stdio.h>
#include <stdarg.h>
#include <vsprintf.h>
#include <fos.h>
#include <fs.h>

int sprintf(char *str, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  int i = vsprintf(str, fmt, args);
  va_end(args);
  return i;
}

char printbuf[256];

fd_t tty=0;

int printf(const char *fmt, ...)
{
  if(!tty)
    while(!(tty = open("/dev/tty", 0))) sched_yield();

  va_list args;
  va_start(args, fmt);
  size_t i = vsprintf(printbuf, fmt, args);
  va_end(args);

  return write(tty, printbuf, i);
}
