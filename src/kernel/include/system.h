/*
  kernel/include/system.h
  Copyright (C) 2005-2007 Oleg Fedorov

  xchg() взят из linux-2.6.17
*/

#ifndef _SYSTEM_H
#define _SYSTEM_H

#include <types.h>

#define BASE_TSK_SEL 0x38
#define SEL_N(SEL) ((SEL)/8)

static inline void sched_yield()
{
  __asm__ __volatile__ ("ljmp $0x40, $0");
}

static inline u16_t str()
{
  u16_t tr;
  __asm__ __volatile__ ("str %0":"=a" (tr));
  return tr;
}

static inline u16_t curPID()
{
  return (str() - BASE_TSK_SEL) / 0x08;
}

static inline u32_t load_cr3()
{
  u32_t cr3;
  __asm__ __volatile__ ("movl %%cr3, %%eax":"=a" (cr3));
  return cr3;
}

static inline void ltr(u16_t tss_selector)
{
  __asm__ __volatile__ ("ltr %%ax \n"::"a" (tss_selector));
}

static inline void lldt(u16_t ldt)
{
  __asm__ __volatile__ ("lldt %0"::"a" (ldt));
}

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

#define cmpxchg(ptr,o,n)						\
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

res_t send(struct message *message);
res_t send_async(struct message *message);
void receive(struct message *message);
void reply(struct message *message);
res_t forward(struct message *message, pid_t pid);

#endif
