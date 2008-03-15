/*
  include/fos/fos.h
  Copyright (C) 2007 Oleg Fedorov
 */

#ifndef _FOS_H
#define _FOS_H

#include <types.h>
#include <fos/syscall.h>

#ifdef iKERNEL
#include <fos/system.h>
#endif

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

static inline void mask_interrupt(u32_t int_num)
{
  sys_call(_FOS_MASK_INTERRUPT, int_num);
}

static inline void unmask_interrupt(u32_t int_num)
{
  sys_call(_FOS_UNMASK_INTERRUPT, int_num);
}

asmlinkage u32_t kill(tid_t tid);
asmlinkage tid_t exec(const char * filename, const char * args);
asmlinkage tid_t exece(const char * filename, const char * args, const char * envp);

#ifndef iKERNEL

#define THREAD(thread) int _ ## thread();				\
  void thread () {							\
    exit (_ ## thread());						\
  }									\
  int _ ## thread()

#endif

asmlinkage tid_t thread_create(off_t eip);

asmlinkage res_t interrupt_attach(u8_t n);
asmlinkage res_t interrupt_detach(u8_t n);

asmlinkage int resmgr_attach(const char *pathname);

asmlinkage size_t dmesg(char *buf, size_t count);

asmlinkage u32_t uptime();
asmlinkage u32_t alarm(u32_t ticks); /* сообщение придет  через ticks */
asmlinkage u32_t alarm2(u32_t ticks);  /* сообщение придет, когда ticks < uptime() */
asmlinkage tid_t my_tid();

#endif
