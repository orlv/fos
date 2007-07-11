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

extern "C" int printk(const char *fmt, ...)
{
  extern TTY *stdout;
  int i = 0;
  if (stdout){
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

#include <hal.h>

fd_t tty = 0;
tid_t resolve(char *name);

int printf(const char *fmt, ...)
{
  return 0;
  if(!tty)
    if(!(tty = open("/dev/tty", 0)))
      return 0;

  va_list args;
  va_start(args, fmt);
  char *printbuf = new char[2048];
  
  size_t i = vsprintf(printbuf, fmt, args);
  va_end(args);
  printbuf[i] = 0;
 
  return write(tty, printbuf, i);
}

#if 0
int printf(const char *fmt, ...)
{
  if(!tty)
    if(!(tty = resolve("/dev/tty")))
      return 0;

  int i = 0;
  va_list args;
  va_start(args, fmt);
  fs_message *m = new fs_message;
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
  delete m;

  return i;
}
#endif
