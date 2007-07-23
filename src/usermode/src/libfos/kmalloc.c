/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos/message.h>
#include <fos/procman.h>

void * kmalloc(size_t size, u32_t flags)
{
  struct message msg;
  msg.a0 = MM_CMD_MEM_ALLOC;
  msg.a1 = size;
  msg.a2 = flags;
  msg.send_size = 0;
  msg.recv_size = 0;
  msg.tid = SYSTID_MM;
  if(send(&msg) == RES_SUCCESS)
    return (void *) msg.a0;
  else
    return 0;
}
