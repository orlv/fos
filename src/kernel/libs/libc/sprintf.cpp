/*
    kernel/lib/sprintf.c
    Copyright (C) 2006-2007 Oleg Fedorov
*/

#include <stdio.h>

int sprintf(char *str, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  int i = vsprintf(str, fmt, args);
  va_end(args);
  return i;
}
