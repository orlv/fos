/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos/message.h>
#include <fos/procman.h>

asmlinkage tid_t thread_create(off_t eip)
{
  struct message msg;
  msg.a0 = PROCMAN_CMD_CREATE_THREAD;
  msg.a1 = eip;
  msg.send_size = 0;
  msg.recv_size = 0;
  msg.flags = 0;
  msg.tid = SYSTID_PROCMAN;
  if(send(&msg) == RES_SUCCESS)
    return (tid_t) msg.a0;
  else
    return 0;
}
