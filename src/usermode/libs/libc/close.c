/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos/message.h>
#include <fos/fs.h>
#include <stdlib.h>

int close(int fildes)
{
  if(!fildes || fildes < 0)
    return -1;

  fd_t fd = (fd_t) fildes;
  
  struct message msg;
  msg.arg[0] = FS_CMD_CLOSE;
  msg.arg[1] = fd->inode;
  msg.send_size = 0;
  msg.recv_size = 0;
  msg.flags = 0;
  msg.tid = fd->thread;
  
  free((char *) fd->fullname);
  free((fd_t) fildes);

  do_send(&msg);
  
  return 0;
}
