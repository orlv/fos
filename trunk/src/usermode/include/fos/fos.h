/*
  include/fos/fos.h
  Copyright (C) 2007 Oleg Fedorov
 */

#ifndef _FOS_H
#define _FOS_H

#include <types.h>
#include <fos/syscall.h>

static inline void mask_interrupt(u32_t int_num)
{
  sys_call(_FOS_MASK_INTERRUPT, int_num);
}

static inline void unmask_interrupt(u32_t int_num)
{
  sys_call(_FOS_UNMASK_INTERRUPT, int_num);
}

//asmlinkage tid_t resolve(char *name);
asmlinkage u32_t kill(tid_t tid);
asmlinkage tid_t exec(const char * filename);

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