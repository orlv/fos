/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos/syscall.h>

asmlinkage tid_t my_tid()
{
  return sys_call(_FOS_MYTID, 0);
}
