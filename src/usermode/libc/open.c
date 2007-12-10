/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos/message.h>
#include <fos/fs.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <fos/namer.h>

#if 0
int resolve(const char *pathname, char **pathtail)
{
  volatile struct message msg;
  msg.arg[0] = NAMER_CMD_RESOLVE;
  size_t len = strlen(pathname);
  if(len > MAX_PATH_LEN)
    return 0;

  char *pt = malloc(len+1);
  
  msg.send_buf = pathname;
  msg.send_size = len+1;
  msg.recv_buf = pt;
  msg.recv_size = len+1;
  msg.flags = 0;
  msg.tid = SYSTID_NAMER;

  u32_t result = send((struct message *)&msg);
  if(result == RES_SUCCESS && msg.arg[0] && msg.arg[2] == NO_ERR) {
    *pathtail = pt;
    return msg.arg[1];
  } else {
    free(pt);
    *pathtail = 0;
    return 0;
  }
}

int open2(const char *pathname, int flags)
{
  char *tail = malloc(MAX_PATH_LEN);
  tid_t tid;
  if((tid = resolve(pathname, &tail))) {
    struct fd *fd = malloc(sizeof(struct fd));
    fd->thread = tid;
    char *fullname = malloc(strlen(pathname) + 1);
    strcpy(fullname, pathname);
    fd->fullname = fullname;
    fd->name = tail;
    return (int) fd;
  } else {
    free(tail);    
    return -1;
  }
}
#endif

int open(const char *pathname, int flags)
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
    
    return (int) fd;
  } else
    return -1;
}
