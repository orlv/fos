/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos/message.h>
#include <fos/procman.h>

void * kmemmap(offs_t ptr, size_t size)
{
  struct message msg;
  msg.a0 = PROCMAN_CMD_MEM_MAP;
  msg.a1 = ptr;
  msg.a2 = size;
  msg.send_size = 0;
  msg.recv_size = 0;
  msg.tid = SYSTID_PROCMAN;
  if(send(&msg) == RES_SUCCESS)
    return (void *) msg.a0;
  else
    return 0;
}
