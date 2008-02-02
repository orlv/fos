/*
 * Copyright (c) 2008 Sergey Gridassov
 */


#include <stdarg.h>
#include <vsprintf.h>

int snprintf(char *str, size_t size, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  int i = vsnprintf(str, size, fmt, args);
  va_end(args);
  return i;
}
