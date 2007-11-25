/*
  include/sched.h
  Copyright (C) 2007 Oleg Fedorov
 */

#ifndef _SCHED_H
#define _SCHED_H

#include <types.h>
#include <fos/syscall.h>

#ifdef iKERNEL
static inline void sched_yield()
{
  __asm__ __volatile__ ("ljmp $0x40, $0");
}
#else
static inline int sched_yield()
{
  return sys_call(_FOS_SCHED_YIELD, 0);
}
#endif

#endif