/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos/message.h>
#include <fos/procman.h>
#include <sched.h>
#include <stdio.h>
void exit(int status)
{
  struct message msg;
  msg.arg[0] = PROCMAN_CMD_EXIT;
  msg.send_size = 0;
  msg.recv_size = 0;
  msg.flags = 0;
  msg.tid = SYSTID_PROCMAN;
  send(&msg);
  while(1);
}
