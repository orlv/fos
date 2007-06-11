/*
  Copiright (C) 2007 Oleg Fedorov
 */

#ifndef FOS_H
#define FOS_H

#include <types.h>

#define  _FOS_SEND              1
#define  _FOS_RECEIVE           2
#define  _FOS_REPLY             3
#define  _FOS_MASK_INTERRUPT    4
#define  _FOS_UNMASK_INTERRUPT  5

#define PAGE_SIZE 0x1000

static inline void sys_call(u32_t arg1, volatile u32_t arg2)
{
  asm volatile ("int $0x30"::"b" (arg1), "c"(arg2));
}

struct message {
  void *send_buf;
  size_t send_size;
  void *recv_buf;
  size_t recv_size;
  tid_t tid;
} __attribute__ ((packed));

static inline void receive(volatile struct message *msg)
{
  sys_call(_FOS_RECEIVE, (u32_t) msg);
}

static inline void send(volatile struct message *msg)
{
  sys_call(_FOS_SEND, (u32_t) msg);
}

static inline void reply(volatile struct message *msg)
{
  sys_call(_FOS_REPLY, (u32_t) msg);
}

static inline void mask_interrupt(u32_t int_num)
{
  sys_call(_FOS_MASK_INTERRUPT, int_num);
}

static inline void unmask_interrupt(u32_t int_num)
{
  sys_call(_FOS_UNMASK_INTERRUPT, int_num);
}

asmlinkage tid_t resolve(char *name);

asmlinkage void exit();
asmlinkage void kill(tid_t tid);
asmlinkage res_t exec(string filename);

asmlinkage void *kmemmap(offs_t ptr, size_t size);
asmlinkage void *kmalloc(size_t size);

asmlinkage tid_t thread_create(off_t eip);

asmlinkage res_t interrupt_attach(u8_t n);
asmlinkage res_t interrupt_detach(u8_t n);

#endif
