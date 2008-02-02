/*
 * Copyright (c) 2008 Sergey Gridassov
 *
 * Не используйте эту функцию! Используйте snprintf.
 */


#include <stdarg.h>
#include <vsprintf.h>

int sprintf(char *str, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  int i = vsprintf(str, fmt, args);
  va_end(args);
  return i;
}
