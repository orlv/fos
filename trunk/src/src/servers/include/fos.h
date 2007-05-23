/*
  Copiright (C) 2007 Oleg Fedorov
 */

#ifndef FOS_H
#define FOS_H

#include <types.h>

#define _FOS_SEND    1
#define _FOS_RECEIVE 2
#define _FOS_REPLY   3

#define PAGE_SIZE 0x1000

static inline void sys_call(u32_t arg1, u32_t arg2)
{
  asm volatile ("int $0x30"::"b" (arg1), "c"(arg2));
}

struct msg {
  volatile void *send_buf;
  volatile unsigned long send_size;
  volatile void *recv_buf;
  volatile unsigned long recv_size;
  volatile unsigned long pid;
} __attribute__ ((packed));

static inline void receive(struct msg *msg)
{
  sys_call(_FOS_RECEIVE, (u32_t) msg);
}

static inline void send(struct msg *msg)
{
  sys_call(_FOS_SEND, (u32_t) msg);
}

static inline void reply(struct msg *msg)
{
  sys_call(_FOS_REPLY, (u32_t) msg);
};

asmlinkage void exit();
asmlinkage void kill(pid_t pid);
asmlinkage res_t exec(string filename);

asmlinkage void *kmemmap(offs_t ptr, size_t size);
asmlinkage void *kmalloc(size_t size);

#endif
