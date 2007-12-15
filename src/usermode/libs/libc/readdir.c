/* 
  Copyright (C) 2007 Sergey Gridassov
 */
  
#include <dirent.h>
#include <fos/message.h>
#include <fos/fs.h>
struct dirent *readdir(DIR *dir) {
  struct dirfd *fd = (struct dirfd *) dir;
  if(fd->offset >= fd->inodes)
    return NULL;
  struct message msg;

  msg.arg[0] = FS_CMD_DIRREAD;
  msg.arg[1] = fd->inode;
  msg.arg[2] = fd->offset;
  msg.flags = 0;
  msg.send_size = 0;
  msg.recv_size = sizeof(struct dirent);
  msg.tid = fd->thread;
  msg.recv_buf = &fd->ent;
  if(do_send(&msg) != RES_SUCCESS)
    return NULL;
  if(msg.arg[2] == ERR_UNKNOWN_CMD || msg.arg[2] == ERR_EOF)
    return NULL;
  fd->ent.d_off = fd->offset;
  fd->offset ++;
  return &fd->ent;
}
