/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos/message.h>
#include <fos/fs.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

int open(const char *pathname, int flags, ...)
{
  volatile struct message msg;
  msg.arg[0] = FS_CMD_ACCESS;
  size_t len = strlen(pathname);
  if(len > MAX_PATH_LEN)
    return 0;

  msg.send_buf = pathname;
  msg.send_size = len+1;
  msg.recv_size = 0;
  msg.flags = 0;
  msg.tid = SYSTID_NAMER;

  u32_t result = send((struct message *)&msg);
  if(result == RES_SUCCESS && msg.arg[0] && msg.arg[2] == NO_ERR) {
    struct fd *fd = (struct fd *) malloc(sizeof(struct fd));


    if(!(flags & O_FOS_DNTCOPY_NAME)) {
      fd->fullname = (const char *) malloc(len+1);
      strcpy((char *)fd->fullname, pathname);
    } else
      fd->fullname = pathname;

    fd->thread = msg.tid;
    fd->inode = msg.arg[0];
    fd->buf_size = msg.arg[1];
    fd->file_size = msg.arg[3];
    return (int) fd;
  } else
    return -1;
}
