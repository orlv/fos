/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos/message.h>
#include <fos/procman.h>

int kfree(off_t ptr)
{
  struct message msg;
  msg.a0 = MM_CMD_MEM_FREE;
  msg.a1 = ptr;
  msg.send_size = 0;
  msg.recv_size = 0;
  msg.tid = SYSTID_MM;
  if(send(&msg) == RES_SUCCESS)
    return msg.a0;
  else
    return 0;
}
