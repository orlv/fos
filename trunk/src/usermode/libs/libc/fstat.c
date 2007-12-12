/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos/message.h>
#include <fos/fs.h>
#include <string.h>
#include <sys/stat.h>

int fstat(int fildes, struct stat *buf)
{
  fd_t fd = (fd_t) fildes;
  if(!fildes || fildes == -1 || !fd->thread)
    return -1;

  struct message msg;
  msg.arg[0] = FS_CMD_FSTAT;
  msg.arg[1] = fd->inode;
  msg.send_size = 0;
  msg.recv_buf = buf;
  msg.recv_size = sizeof(struct stat);
  msg.flags = 0;
  msg.tid = fd->thread;

  if(send((struct message *)&msg) == RES_SUCCESS &&
     msg.arg[2] == NO_ERR)
    return 0;
  else
    return -1;
}
