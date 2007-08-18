/*
  lib/printk.cpp
  Copyright (C) 2006 Oleg Fedorov
*/

#include <fos/printk.h>
#include <fos/drivers/char/tty/tty.h>
#include <fos/hal.h>
#include <stdarg.h>
#include <vsprintf.h>

asmlinkage int printk(const char *fmt, ...)
{
  extern TTY *stdout;
  int i = 0;
  if (stdout) {
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
