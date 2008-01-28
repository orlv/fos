/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos/message.h>
#include <fos/fs.h>
#include <fcntl.h>
#include <stdlib.h>

ssize_t read(int fildes, void *buf, size_t nbyte)
{
  fd_t fd = (fd_t) fildes;
  if(!fildes || fildes < 0 || !fd->thread)
    return -1;

  struct message msg;
  size_t offset = 0;

  while(1) {
    msg.arg[0] = FS_CMD_READ;
    msg.arg[1] = fd->inode;
    msg.arg[2] = fd->offset;
    msg.flags = 0;
    msg.send_size = 0;
    msg.recv_size = nbyte - offset; /* учитываем уже прочитанные данные */
    msg.tid = fd->thread;
    msg.recv_buf = &((char *)buf)[offset];

    if(do_send(&msg) != RES_SUCCESS) /* получатель не найден, запрос не отправлен! */
      return -1;

    if(msg.arg[2] == ERR_UNKNOWN_CMD)
      return -2;

    if(msg.arg[2] == ERR_TIMEOUT) {
      fildes = open(fd->fullname,  O_FOS_DNTCOPY_NAME);
      if(fildes && (fildes > 0)) {
	fd_t fd1 = (fd_t) fildes;
	fd->inode = fd1->inode;
	fd->thread = fd1->thread;
	fd->buf_size = fd1->buf_size;
	free(fd1);
	continue;
      } else
	return -3;
    }

    fd->file_size = msg.arg[1];

    offset += msg.arg[0];
    fd->offset = offset;
    
    if((msg.arg[2] == ERR_EOF) || offset >= nbyte) /* мы достигли конца файла, или прочитали всё что хотели */
      return offset;
  }
}
