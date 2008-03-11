/*
  fos/system.h
  Copyright (C) 2007-2008 Oleg Fedorov
*/

#ifndef _FOS_PREEMPT_H
#define _FOS_PREEMPT_H

#include <types.h>


static inline void preempt_reset()
{
  extern volatile size_t preempt_count;
  preempt_count = 1;
}
  
static inline void preempt_disable()
{
  extern volatile size_t preempt_count;
  preempt_count++;
}

static inline void preempt_enable_no_resched()
{
  extern volatile size_t preempt_count;
  if(preempt_count) preempt_count--;
}

static inline void preempt_enable()
{
  extern volatile size_t preempt_count;
  if(preempt_count) preempt_count--;
  /*  if(!preempt_count && пропущено_переключение)
      sched_yield();*/
}

static inline bool preempt_status()
{
  extern volatile size_t preempt_count;
  return preempt_count == 0;
}

static inline void preempt_on()
{
  extern volatile size_t preempt_count;
  preempt_count = 0;
}

#endif
