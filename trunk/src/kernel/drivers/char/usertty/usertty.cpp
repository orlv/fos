/*
  drivers/char/usertty/usertty.cpp
  Copyright (C) 2007 Oleg Fedorov
*/

#include "usertty.h"
#include <hal.h>
#include <fs.h>
#include <string.h>

//tid_t resolve(char *name);

USERTTY::USERTTY():Tinterface()
{
  tty = 0;
}

size_t USERTTY::write(off_t offset, const void *buf, size_t count)
{
  if(count > FS_CMD_LEN)
    count = FS_CMD_LEN - 1;

  if(!tty)
    while(!(tty = open("/dev/tty", 0)));

  //fs_message *ttymsg = new fs_message;
  //ttymsg->data3.cmd = FS_CMD_WRITE;
  //strncpy(ttymsg->data3.buf, (const char *)buf, count);
  //ttymsg->data3.buf[count] = 0;
  /*  volatile struct message msg;
  msg.a0 = FS_CMD_WRITE;
  msg.send_buf = (char *)buf;
  msg.send_size = count + 1;
  msg.recv_size = 0;
  msg.tid = tty;
  send((struct message *)&msg);*/

  return -1; //write(tty, buf, count);
}

#if 0
size_t USERTTY::write(off_t offset, const void *buf, size_t count)
{
  if(count > FS_CMD_LEN)
    count = FS_CMD_LEN - 1;

  if(!tty)
    while(!(tty = resolve("/dev/tty")));

  fs_message *ttymsg = new fs_message;
  ttymsg->data3.cmd = FS_CMD_WRITE;
  strncpy(ttymsg->data3.buf, (const char *)buf, count);
  ttymsg->data3.buf[count] = 0;
  volatile struct message msg;
  msg.send_buf = msg.recv_buf = ttymsg;
  msg.send_size = 8 + count + 1;
  msg.recv_size = sizeof(unsigned long);
  msg.tid = tty;
  send((struct message *)&msg);

  return count;
}
#endif
