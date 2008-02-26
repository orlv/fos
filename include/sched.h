/*
  include/sched.h
  Copyright (C) 2007-2008 Oleg Fedorov
 */

#ifndef _SCHED_H
#define _SCHED_H

#ifdef iKERNEL
void sched_yield();
#else

#include <fos/syscall.h>

static inline int sched_yield()
{
  return sys_call(_FOS_SCHED_YIELD, 0);
}
#endif

#endif
