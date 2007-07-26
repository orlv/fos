/*
  lib/fs.cpp
  Copyright (C) 2006-2007 Oleg Fedorov
*/

#include <fos/hal.h>
#include <fos/fs.h>
#include <fos/printk.h>
#include <fos/namer.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

asmlinkage ssize_t read(int fildes, void *buf, size_t nbyte)
{
  fd_t fd = (fd_t) fildes;
  if(!fildes || fildes == -1 || !fd->thread)
    return -1;

  message msg;
  size_t offset = 0;

  if(fd->buf_size < nbyte)
    msg.recv_size = fd->buf_size;
  else
    msg.recv_size = nbyte;

  do{
    msg.a0 = FS_CMD_READ;
    msg.send_size = 0;
    msg.a1 = fd->inode;
    msg.a2 = fd->offset;
    msg.tid = fd->thread;

    msg.recv_buf = &((char *)buf)[offset];

    if(do_send(&msg) != RES_SUCCESS) /* получатель не найден! */
      return -1;

    if(msg.a2 == ERR_UNKNOWN_CMD)
      return -2;
    
    offset += msg.a0;
    fd->offset = offset;
  
    if((msg.a2 == ERR_EOF) || offset >= nbyte)
      return offset;

    if(offset + msg.send_size > nbyte)
      msg.send_size = nbyte - offset;
  } while(1);
}

asmlinkage ssize_t write(int fildes, const void *buf, size_t nbyte)
{
  fd_t fd = (fd_t) fildes;  
  if(!fildes || fildes == -1 || !fd->thread)
    return -1;

  message msg;
  size_t offset = 0;

  if(fd->buf_size < nbyte)
    msg.send_size = fd->buf_size;
  else
    msg.send_size = nbyte;

  do{
    msg.a0 = FS_CMD_WRITE;
    msg.recv_size = 0;
    msg.a1 = fd->inode;
    msg.a2 = fd->offset;
    msg.tid = fd->thread;

    msg.send_buf = &((char *)buf)[offset];

    if(do_send(&msg) != RES_SUCCESS) /* получатель не найден! */
      return -1;

    if(msg.a2 == ERR_UNKNOWN_CMD)
      return -2;
    
    offset += msg.a0;
    fd->offset = offset;

    if((msg.a2 == ERR_EOF) || offset >= nbyte)
      return offset;
   
    
    if(offset + msg.send_size > nbyte)
      msg.send_size = nbyte - offset;
  } while(1);
}

asmlinkage int open(const char *pathname, int flags)
{
  volatile struct message msg;
  msg.a0 = FS_CMD_ACCESS;
  size_t len = strlen(pathname);
  if(len > MAX_PATH_LEN)
    return 0;

  msg.send_buf = pathname;
  msg.send_size = len+1;
  msg.recv_size = 0;
  msg.tid = SYSTID_NAMER;

  u32_t result = send((message *)&msg);

  if(result == RES_SUCCESS && msg.a0 && msg.a2 == NO_ERR) {
    struct fd *fd = new struct fd;
    fd->thread = msg.tid;
    fd->inode = msg.a0;
    fd->buf_size = msg.a1;
    return (int) fd;
  } else
    return -1;
}

asmlinkage int close(int fildes)
{
  if(!fildes || fildes == -1)
    return -1;

  delete (fd_t) fildes;
  return 0;
}

asmlinkage int resmgr_attach(const char *pathname)
{
  if(!pathname)
    return 0;

  message msg;
  msg.a0 = NAMER_CMD_ADD;
  size_t len = strlen(pathname);
  if(len+1 > MAX_PATH_LEN)
    return 0;

  msg.send_buf = pathname;
  msg.send_size = len+1;
  msg.recv_size = 0;
  msg.tid = SYSTID_NAMER;
  return send((message *)&msg);
}

asmlinkage int stat(const char *path, struct stat *buf)
{
  size_t len = strlen(path);
  if(len > MAX_PATH_LEN)
    return 0;

  volatile struct message msg;
  msg.a0 = FS_CMD_STAT;
  msg.send_buf = path;
  msg.send_size = len+1;
  msg.recv_buf = buf;
  msg.recv_size = sizeof(struct stat);
  msg.tid = SYSTID_NAMER;

  if(send((struct message *)&msg) == RES_SUCCESS &&
     msg.a2 == NO_ERR)
    return 0;
  else
    return -1;
}

asmlinkage int fstat(int fildes, struct stat *buf)
{
  fd_t fd = (fd_t) fildes;
  if(!fildes || fildes == -1 || !fd->thread)
    return -1;

  struct message msg;
  msg.a0 = FS_CMD_FSTAT;
  msg.a1 = fd->inode;
  msg.send_size = 0;
  msg.recv_buf = buf;
  msg.recv_size = sizeof(struct stat);
  msg.tid = fd->thread;

  if(send((struct message *)&msg) == RES_SUCCESS &&
     msg.a2 == NO_ERR)
    return 0;
  else
    return -1;
}
