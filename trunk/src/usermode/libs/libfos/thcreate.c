/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos/message.h>
#include <fos/procman.h>

asmlinkage tid_t thread_create(off_t eip, u32_t arg)
{
  struct message msg;
  msg.arg[0] = PROCMAN_CMD_CREATE_THREAD;
  msg.arg[1] = eip;
  msg.arg[2] = arg;
  msg.send_size = 0;
  msg.recv_size = 0;
  msg.flags = 0;
  msg.tid = SYSTID_PROCMAN;
  if(send(&msg) == RES_SUCCESS)
    return (tid_t) msg.arg[0];
  else
    return 0;
}
