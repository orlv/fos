/*
  fos/system.h
  Copyright (C) 2007-2008 Oleg Fedorov
*/

#ifndef _FOS_PREEMPT_H
#define _FOS_PREEMPT_H

#include <types.h>
#include <c++/atomic.h>

static inline void preempt_reset()
{
  extern atomic_t preempt_count;
  preempt_count.set(1);
}
  
static inline void preempt_disable()
{
  extern atomic_t preempt_count;
  preempt_count.inc();
}

static inline void preempt_enable_no_resched()
{
  extern atomic_t preempt_count;
  if(preempt_count.value()) preempt_count.dec();
}

static inline void preempt_enable()
{
  extern atomic_t preempt_count;
  if(preempt_count.value()) preempt_count.dec();
  /*  if(!preempt_count.value() && пропущено_переключение)
      sched_yield();*/
}

static inline bool preempt_status()
{
  extern atomic_t preempt_count;
  return preempt_count.value() == 0;
}

static inline void preempt_on()
{
  extern atomic_t preempt_count;
  preempt_count.set(0);
}

#endif
