/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos/syscall.h>

asmlinkage u32_t uptime()
{
  return sys_call(_FOS_UPTIME, 0);
}
