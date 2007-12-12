/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos/syscall.h>

asmlinkage u32_t alarm2(u32_t ticks)
{
  return sys_call2(_FOS_ALARM, 0, ticks);
}
