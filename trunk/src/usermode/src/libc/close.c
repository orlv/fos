/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos/message.h>
#include <fos/syscall.h>
#include <fos/fs.h>
#include <stdlib.h>

int close(int fildes)
{
  if(!fildes || fildes == -1)
    return -1;

  free((fd_t) fildes);
  return 0;
}
