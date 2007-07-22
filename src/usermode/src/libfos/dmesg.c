/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos/message.h>
#include <fos/procman.h>

asmlinkage size_t dmesg(char *buf, size_t count)
{
  struct message msg;
  msg.a0 = PROCMAN_CMD_DMESG;
  msg.recv_buf = buf;
  msg.recv_size = count;
  msg.send_size = 0;
  msg.tid = SYSTID_PROCMAN;
  if(send(&msg) == RES_SUCCESS)
    return msg.recv_size;
  else
    return 0;
}
