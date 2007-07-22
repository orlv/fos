/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos/syscall.h>

asmlinkage u32_t alarm(u32_t ticks)
{
  return sys_call(_FOS_ALARM, ticks);
}
