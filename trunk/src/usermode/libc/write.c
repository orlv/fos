/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos/message.h>
#include <fos/fs.h>

asmlinkage ssize_t write(int fildes, const void *buf, size_t nbyte)
{
  fd_t fd = (fd_t) fildes;  
  if(!fildes || fildes == -1 || !fd->thread)
    return -1;

  struct message msg;
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

    if((msg.a2 == ERR_EOF) || offset >= nbyte)
      return offset;
   
    
    if(offset + msg.send_size > nbyte)
      msg.send_size = nbyte - offset;
  } while(1);
}