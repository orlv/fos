/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos/message.h>
#include <fos/procman.h>
#include <fos/namer.h>
#include <string.h>

int resmgr_attach(const char *pathname)
{
  if(!pathname)
    return 0;

  struct message msg;
  msg.arg[0] = NAMER_CMD_ADD;
  size_t len = strlen(pathname);
  if(len+1 > MAX_PATH_LEN)
    return 0;

  msg.send_buf = pathname;
  msg.send_size = len+1;
  msg.recv_size = 0;
  msg.flags = 0;
  msg.tid = SYSTID_NAMER;
  if(send(&msg) == RES_SUCCESS)
    return msg.arg[0];
  else
    return 0;
}
