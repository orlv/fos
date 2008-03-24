/*
  Copyright (C) 2007 Oleg Fedorov
  Serial I/O (c) 2007 Gridassov Sergey
 */

#include <fos/fos.h>
#include <fos/fs.h>
#include <fos/message.h>
#include <stdlib.h>
#include "tty.h"

#define RECV_BUF_SIZE 2048

asmlinkage int main()
{
  tty_init();

  struct message msg;
  char *buffer = malloc(RECV_BUF_SIZE);

  if (resmgr_attach("/dev/tty") != RES_SUCCESS)
    return -1;

  while (1) {
    msg.tid = 0;
    msg.flags = 0;
    msg.recv_size = RECV_BUF_SIZE;
    msg.recv_buf = buffer;
    receive(&msg);
    switch (msg.arg[0]) {
    case FS_CMD_ACCESS:
      msg.arg[0] = 1;
      msg.arg[1] = RECV_BUF_SIZE;
      msg.arg[2] = NO_ERR;
      break;

    case FS_CMD_WRITE:
      msg.arg[0] = tty_write(0 /*msg.arg[2] */ , buffer, msg.recv_size);
      msg.arg[2] = NO_ERR;
      break;

    default:
      msg.arg[0] = 0;
      msg.arg[2] = ERR_UNKNOWN_METHOD;
    }

    msg.send_size = 0;
    reply(&msg);
  }
  return 0;
}
