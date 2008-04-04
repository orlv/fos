/*
  lib/printk.cpp
  Copyright (C) 2006 Oleg Fedorov
*/

#include <fos/printk.h>
#include <fos/drivers/tty/tty.h>
#include <fos/system.h>
#include <stdio.h>

int printk(const char *fmt, ...)
{
  extern TTY *stdout;
  int i = 0;
  if (stdout) {
    va_list args;
    va_start(args, fmt);
    char *printbuf = new char[2000];
    i = vsprintf(printbuf, fmt, args);
    va_end(args);
    system->preempt.disable();
    stdout->write(0, printbuf, i);
    system->preempt.enable();
    delete printbuf;
  }
  return i;
}
