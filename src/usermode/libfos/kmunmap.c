/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos/message.h>
#include <fos/procman.h>

int kmunmap(off_t start, size_t lenght)
{
  struct message msg;
  msg.a0 = MM_CMD_MUNMAP;
  msg.a1 = start;
  msg.a2 = lenght;
  msg.send_size = 0;
  msg.recv_size = 0;
  msg.tid = SYSTID_MM;
  if(send(&msg) == RES_SUCCESS)
    return msg.a0;
  else
    return 0;
}
