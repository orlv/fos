/*
  Copyright (C) 2007 Oleg Fedorov

  xchg взят из linux-2.6.17
 */

#ifndef _XCHG_H
#define _XCHG_H

#include <types.h>

struct __xchg_dummy { unsigned long a[100]; };
#define __xg(x) ((struct __xchg_dummy *)(x))
#define xchg(ptr,v) ((__typeof__(*(ptr)))__xchg((unsigned long)(v),(ptr),sizeof(*(ptr))))

static inline unsigned long __xchg(unsigned long x, volatile void * ptr, int size)
{
        switch (size) {
                case 1:
                        __asm__ __volatile__("xchgb %b0,%1"
                                :"=q" (x)
                                :"m" (*__xg(ptr)), "0" (x)
                                :"memory");
                        break;
                case 2:
                        __asm__ __volatile__("xchgw %w0,%1"
                                :"=r" (x)
                                :"m" (*__xg(ptr)), "0" (x)
                                :"memory");
                        break;
                case 4:
                        __asm__ __volatile__("xchgl %0,%1"
                                :"=r" (x)
                                :"m" (*__xg(ptr)), "0" (x)
                                :"memory");
                        break;
        }
        return x;
}

#define _cmpxchg(ptr,o,n)						\
  ((__typeof__(*(ptr)))__cmpxchg((ptr),(unsigned long)(o),		\
				 (unsigned long)(n),sizeof(*(ptr))))

static inline unsigned long __cmpxchg(volatile void *ptr, unsigned long old, unsigned long n, int size)
{
  unsigned long prev;
  switch (size) {
  case 1:
    __asm__ __volatile__("cmpxchgb %b1,%2"
			 : "=a"(prev)
			 : "q"(n), "m"(*__xg(ptr)), "0"(old)
			 : "memory");
    return prev;
  case 2:
    __asm__ __volatile__("cmpxchgw %w1,%2"
			 : "=a"(prev)
			 : "r"(n), "m"(*__xg(ptr)), "0"(old)
			 : "memory");
    return prev;
  case 4:
    __asm__ __volatile__("cmpxchgl %1,%2"
			 : "=a"(prev)
			 : "r"(n), "m"(*__xg(ptr)), "0"(old)
			 : "memory");
    return prev;
  }
  return old;
}


#endif
