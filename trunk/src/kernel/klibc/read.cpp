/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos/message.h>
#include <fos/fs.h>

ssize_t read(int fildes, void *buf, size_t nbyte)
{
  fd_t fd = (fd_t) fildes;
  if(!fildes || fildes == -1 || !fd->thread)
    return -1;

  struct message msg;
  size_t offset = 0;

  if(fd->buf_size < nbyte)
    msg.recv_size = fd->buf_size;
  else
    msg.recv_size = nbyte;

  do{
    msg.arg[0] = FS_CMD_READ;
    msg.send_size = 0;
    msg.arg[1] = fd->inode;
    msg.arg[2] = fd->offset;
    msg.flags = 0;
    msg.tid = fd->thread;

    msg.recv_buf = &((char *)buf)[offset];

    if(do_send(&msg) != RES_SUCCESS) /* получатель не найден! */
      return -1;

    if(msg.arg[2] == ERR_UNKNOWN_CMD)
      return -2;
    
    offset += msg.arg[0];
    fd->offset = offset;
    
    if((msg.arg[2] == ERR_EOF) || offset >= nbyte)
      return offset;

    if(offset + msg.send_size > nbyte)
      msg.send_size = nbyte - offset;
  } while(1);
}
