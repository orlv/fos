/*
  (C)  2007 Oleg Fedorov
*/

#ifndef _MUTEX_H
#define _MUTEX_H

#include <types.h>
#include <asm/xchg.h>
typedef volatile u32_t mutex_t;

static inline u32_t __atom_mutex_set_val(mutex_t *m, volatile u32_t val) {
  __asm__ __volatile__("lock xchgl %%ecx, (%%eax)":"=c"(val):"a"(m), "c"(val):"memory");
  return val;
}

static inline void mutex_init(mutex_t *m)
{
  *m = 0;
}

static inline void mutex_lock(mutex_t *m)
{
  while(__atom_mutex_set_val(m, 1));
}

static inline int mutex_unlock(mutex_t *m)
{
  return __atom_mutex_set_val(m, 0) == 1;
}

static inline int mutex_try_lock(mutex_t *m)
{
  return __atom_mutex_set_val(m, 1) == 0;
}

static inline int mutex_is_locked(mutex_t *m)
{
  return *m;
}

#endif

