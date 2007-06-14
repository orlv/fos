/*
    lib/stdio.c
    Copyright (C) 2006-2007 Oleg Fedorov
*/

#include <stdio.h>
#include <stdarg.h>
#include <vsprintf.h>
#include <fos.h>
#include <fs.h>

#define PID_TTY 4

int sprintf(char *str, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  int i = vsprintf(str, fmt, args);
  va_end(args);
  return i;
}

char printbuf[256];

extern tid_t tty;

int printf(const char *fmt, ...)
{
  if(!tty)
    while(!(tty = resolve("/dev/tty")));
  
  int i = 0;
  va_list args;
  va_start(args, fmt);
  union fs_message *m = (union fs_message *) printbuf;
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
