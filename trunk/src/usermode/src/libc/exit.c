/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos/message.h>
#include <fos/syscall.h>
#include <fos/procman.h>

void exit(int status)
{
  struct message msg;
  msg.a0 = PROCMAN_CMD_EXIT;
  msg.send_size = 0;
  msg.recv_size = 0;
  msg.tid = SYSTID_PROCMAN;
  send(&msg);
  while(1);
}
