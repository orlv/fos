/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos/message.h>
#include <fos/syscall.h>
#include <fos/fs.h>
#include <fos/procman.h>
#include <string.h>

tid_t exec(const char * filename)
{
  size_t len = strlen(filename);
  if(len+1 > MAX_PATH_LEN)
    return 0;

  struct message msg;
  msg.a0 = PROCMAN_CMD_EXEC;
  msg.send_buf = filename;
  msg.send_size = len + 1;
  msg.recv_size = 0;
  msg.tid = SYSTID_PROCMAN;

  if(send(&msg) == RES_SUCCESS)
    return (tid_t) msg.a0;
  else
    return 0;
}
