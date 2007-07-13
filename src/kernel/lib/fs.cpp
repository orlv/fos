/*
  namer/namer.cpp
  Copyright (C) 2006-2007 Oleg Fedorov
*/

#include <hal.h>
#include <string.h>
#include <fs.h>

int close(fd_t fd)
{
  if(!fd)
    return -1;

  delete fd;
  return 0;
}

size_t read(fd_t fd, void *buf, size_t count)
{
  if(!fd || !fd->thread)
    return 0;

  message msg;
  msg.a0 = FS_CMD_READ;
  msg.recv_size = count;
  msg.recv_buf = buf;
  msg.send_size = 0;
  msg.a1 = fd->id;
  msg.a2 = fd->offset;
  msg.tid = fd->thread;

  do{
    switch(send(&msg)){
    case RES_SUCCESS:
      return msg.recv_size;
      
    case RES_FAULT2: /* очередь получателя переполнена, обратимся чуть позже */
      continue;
      
    default:
      return 0;
    }
  }while(1);
}

size_t write(fd_t fd, void *buf, size_t count)
{
  if(!fd || !fd->thread)
    return 0;

  message msg;
  msg.a0 = FS_CMD_WRITE;
  msg.recv_size = 0;
  msg.send_buf = buf;
  msg.a1 = fd->id;
  msg.a2 = fd->offset;
  msg.tid = fd->thread;

  do{
    msg.send_size = count;
    
    switch(send(&msg)){
    case RES_SUCCESS:
      return msg.a0;
      
    case RES_FAULT2: /* очередь получателя переполнена, обратимся чуть позже */
      continue;
      
    default:
      return 0;
    }
  }while(1);
}

fd_t open(const char *pathname, int flags)
{
  volatile struct message msg;
  msg.a0 = FS_CMD_ACCESS;
  size_t len = strlen(pathname);
  if(len > MAX_PATH_LEN)
    return 0;

  msg.send_buf = pathname;
  msg.send_size = len+1;
  msg.tid = SYSTID_NAMER;

  u32_t result = send((message *)&msg);
  if(result == RES_SUCCESS && msg.a0) {
    struct fd *fd = new struct fd;
    fd->thread = msg.tid;
    fd->id = msg.a0;
    return fd;
  } else
    return 0;
}

int resmgr_attach(const char *pathname)
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
