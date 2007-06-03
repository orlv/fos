/*
	kernel/include/system.h
	Copyright (C) 2005-2006 Oleg Fedorov
*/

#ifndef _SYSTEM_H
#define _SYSTEM_H

#include <types.h>

#define BASE_TSK_SEL 0x38
#define BASE_TSK_SEL_N 7

inline void sched_yield()
{
  asm volatile ("ljmp $0x40, $0");
}

inline u16_t str()
{
  u16_t tr;
  asm volatile ("str %0":"=a" (tr));
  return tr;
}

inline u16_t curPID()
{
  return (str() - BASE_TSK_SEL) / 0x08;
}

inline u32_t load_cr3()
{
  u32_t cr3;
  asm volatile ("movl %%cr3, %%eax":"=a" (cr3));
  return cr3;
}

inline void ltr(u16_t tss_selector)
{
  asm volatile ("ltr %%ax \n"::"a" (tss_selector));
}

inline void lldt(u16_t ldt)
{
  asm volatile ("lldt %0"::"a" (ldt));
}

res_t send(struct message *message);
res_t send_async(struct message *message);
void receive(struct message *message);
void reply(struct message *message);
res_t forward(struct message *message, pid_t pid);

#endif
