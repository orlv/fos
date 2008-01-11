/*
    lib/printf.c
    Copyright (C) 2006-2007 Oleg Fedorov
*/

#include <stdarg.h>
#include <vsprintf.h>
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
#include <stdio.h>
char printbuf[256];

int printf(const char *fmt, ...)
{
  if(stdout < 1) return 0;
  va_list args;
  va_start(args, fmt);
  size_t i = vsprintf(printbuf, fmt, args);
  va_end(args);

  return write(stdout, printbuf, i);
}
