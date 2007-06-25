/*
	lib/stdio.cpp
	Copyright (C) 2006 Oleg Fedorov
*/

#include <stdio.h>
#include <stdarg.h>
#include <drivers/char/tty/tty.h>
#include <vsprintf.h>
#include <system.h>
#include <hal.h>

int sprintf(char *str, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  int i = vsprintf(str, fmt, args);
  va_end(args);
  return i;
}

char printbuf[2000];

int printk(const char *fmt, ...)
{
  extern TTY *stdout;
  int i = 0;
  if (stdout){
    va_list args;
    va_start(args, fmt);
    hal->mt_disable();
    i = vsprintf(printbuf, fmt, args);
    va_end(args);
    
    *stdout << printbuf;
    hal->mt_enable();
  }
  return i;
}

#include <hal.h>

tid_t tty = 0;
tid_t resolve(char *name);

int printf(const char *fmt, ...)
{
  return 0;
  if(!tty)
    if(!(tty = resolve("/dev/tty")))
      return 0;

  int i = 0;
  va_list args;
  va_start(args, fmt);
  fs_message *m = (fs_message *) printbuf;
  m->data3.cmd = FS_CMD_WRITE;
  i = vsprintf(m->data3.buf, fmt, args);
  va_end(args);

  m->data3.buf[i] = 0;
  volatile struct message msg;
  msg.send_buf = msg.recv_buf = m;
  msg.send_size = 8 + i + 1;
  msg.recv_size = sizeof(unsigned long);
  msg.tid = tty;
  send((struct message *)&msg);

  return i;
}
