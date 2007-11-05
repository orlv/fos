/*
  (C)  2007 Oleg Fedorov
*/

#ifndef _MUTEX_H
#define _MUTEX_H

#include <types.h>
#include <asm/xchg.h>

typedef volatile u8_t mutex_t;

static inline void mutex_init(mutex_t m)
{
  m = 0;
}

static inline void mutex_lock(mutex_t m)
{
  while (xchg(&m, 1) != 0);
}

static inline int mutex_unlock(mutex_t m)
{
  return xchg(&m, 0) == 0;
}

static inline int mutex_try_lock(mutex_t m)
{
  return xchg(&m, 1) == 0;
}

static inline int mutex_is_locked(mutex_t m)
{
  return m;
}

#endif
