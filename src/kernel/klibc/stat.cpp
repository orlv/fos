/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos/message.h>
#include <fos/fs.h>
#include <string.h>
#include <sys/stat.h>

int stat(const char *path, struct stat *buf)
{
  size_t len = strlen(path);
  if(len > MAX_PATH_LEN)
    return 0;

  volatile struct message msg;
  msg.arg[0] = FS_CMD_STAT;
  msg.send_buf = path;
  msg.send_size = len+1;
  msg.recv_buf = buf;
  msg.recv_size = sizeof(struct stat);
  msg.flags = 0;
  msg.tid = SYSTID_NAMER;

  if(send((struct message *)&msg) == RES_SUCCESS &&
     msg.arg[2] == NO_ERR)
    return 0;
  else
    return -1;
}
