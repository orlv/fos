/*
    lib/printf.c
    Copyright (C) 2006-2007 Oleg Fedorov
*/

#include <stdarg.h>
#include <vsprintf.h>
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>

char printbuf[256];
int tty = 0;

int printf(const char *fmt, ...)
{
  if(!tty)
    while((tty = open("/dev/tty", 0)) == -1) sched_yield();

  va_list args;
  va_start(args, fmt);
  size_t i = vsprintf(printbuf, fmt, args);
  va_end(args);

  return write(tty, printbuf, i);
}
