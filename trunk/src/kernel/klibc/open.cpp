/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos/message.h>
#include <fos/fs.h>
#include <string.h>
#include <stdlib.h>

int open(const char *pathname, int flags)
{
  volatile struct message msg;
  msg.a0 = FS_CMD_ACCESS;
  size_t len = strlen(pathname);
  if(len > MAX_PATH_LEN)
    return 0;

  msg.send_buf = pathname;
  msg.send_size = len+1;
  msg.recv_size = 0;
  msg.flags = 0;
  msg.tid = SYSTID_NAMER;

  u32_t result = send((struct message *)&msg);
  if(result == RES_SUCCESS && msg.a0 && msg.a2 == NO_ERR) {
    struct fd *fd = (struct fd *)malloc(sizeof(struct fd));
    fd->thread = msg.tid;
    fd->inode = msg.a0;
    fd->buf_size = msg.a1;
    return (int) fd;
  } else
    return -1;
}
