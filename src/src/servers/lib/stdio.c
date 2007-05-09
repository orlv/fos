/*
    lib/stdio.c
    Copyright (C) 2006-2007 Oleg Fedorov
*/

#include <stdio.h>
#include <stdarg.h>
#include <vsprintf.h>
#include <fos.h>

int sprintf(char *str, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  int i = vsprintf(str, fmt, args);
  va_end(args);
  return i;
}

char printbuf[256];

int printf(const char *fmt, ...)
{
  int i = 0;
  va_list args;
  va_start(args, fmt);
  i = vsprintf(printbuf, fmt, args);
  va_end(args);

  printbuf[i] = 0;
  volatile struct msg msg;
  msg.send_buf = msg.recv_buf = printbuf;
  msg.send_size = i + 1;
  msg.recv_size = 10;
  msg.pid = 4;
  send((struct msg *)&msg);

  return i;
}
