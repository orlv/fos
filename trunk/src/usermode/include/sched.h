/*
  Copiright (C) 2007 Oleg Fedorov
 */

#ifndef _SCHED_H
#define _SCHED_H

#include <types.h>
#include <fos/syscall.h>

static inline int sched_yield()
{
  return sys_call(_FOS_SCHED_YIELD, 0);
}

#endif
