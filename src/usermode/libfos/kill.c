/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos/message.h>
#include <fos/procman.h>

u32_t kill(tid_t tid)
{
  struct message msg;
  msg.a0 = PROCMAN_CMD_KILL;
  msg.a1 = tid;
  msg.send_size = 0;
  msg.recv_size = 0;
  msg.tid = SYSTID_PROCMAN;
  if(send(&msg) == RES_SUCCESS)
    return msg.a0;
  else
    return 0;
}
