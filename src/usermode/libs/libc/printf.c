/*
 * Copyright (C) 2008 Sergey Gridassov
 */

#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
#include <stdio.h>

int printf(const char *fmt, ...)
{
  if(stdout == NULL) return 0;
  va_list args;
  va_start(args, fmt);
  size_t i = vfprintf(stdout, fmt, args);
  va_end(args);

  return i;
}
