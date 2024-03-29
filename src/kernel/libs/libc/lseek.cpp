/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos/fs.h>
#include <unistd.h>

off_t lseek(int fildes, off_t offset, int whence)
{
  fd_t fd = (fd_t) fildes;
  if(!fildes || fildes == -1 || !fd->thread)
    return (off_t)-1;

  switch(whence) {
  case SEEK_SET:
    fd->offset = offset;
    return fd->offset;

  case SEEK_CUR:
    fd->offset += offset;
    return fd->offset;

  case SEEK_END:
    if(fd->file_size - offset >= 0) {
      fd->offset = fd->file_size - offset;
      return fd->offset;
    }
  }
  return (off_t)-1;
}
