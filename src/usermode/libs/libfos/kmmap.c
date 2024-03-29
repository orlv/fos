/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos/message.h>
#include <fos/procman.h>

void *kmmap(void *start, size_t lenght, int flags, off_t phys_start)
{
  struct message msg;
  msg.arg[0] = MM_CMD_MMAP;
  msg.arg[1] = ((off_t)start & ~0xfff) | (flags & 0xfff);
  msg.arg[2] = lenght;
  msg.arg[3] = phys_start;
  msg.send_size = 0;
  msg.recv_size = 0;
  msg.flags = 0;
  msg.tid = SYSTID_MM;
  if(send(&msg) == RES_SUCCESS)
    return (void *) msg.arg[0];
  else
    return 0;
}
