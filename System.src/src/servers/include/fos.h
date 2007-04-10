#ifndef FOS_H
#define FOS_H

#include <types.h>

#define _FOS_SEND    1
#define _FOS_RECEIVE 2
#define _FOS_REPLY   3
#define _FOS_MEM_ALLOC 4
#define _FOS_MEM_MAP 5
//#define _FOS_PRINT   6

#define PAGE_SIZE 0x1000

struct memmap {
  u32_t ptr;
  u32_t size;
} __attribute__ ((packed));

static inline void sys_call(u32_t arg1, u32_t arg2)
{
  asm volatile ("int $0x30"::"b" (arg1), "c"(arg2));
}

/*
static inline void print(char ch)
{
  sys_call(_FOS_PRINT, ch);
}
*/

static inline void *kmemmap(offs_t ptr, size_t size)
{
  volatile struct memmap mmp;
  mmp.ptr = ptr;
  mmp.size = size;
  sys_call(_FOS_MEM_MAP, (u32_t) & mmp);
  return (void *)mmp.ptr;
}

static inline void *kmalloc(size_t size)
{
  volatile u32_t exc = size;
  sys_call(_FOS_MEM_ALLOC, (u32_t) & exc);
  return (void *)exc;
}

struct msg {
  volatile void *send_buf;
  volatile unsigned long send_size;
  volatile void *recv_buf;
  volatile unsigned long recv_size;
  volatile unsigned long pid;
} __attribute__ ((packed));

/*
static inline void prints(const char *str)
{
  while(*str)
    {
      print(*str);
      str++;
    }
}
*/

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
}

#endif
