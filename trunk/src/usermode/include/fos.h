/*
  Copiright (C) 2007 Oleg Fedorov
 */

#ifndef FOS_H
#define FOS_H

#include <types.h>

/* основные системные вызовы -- обмен сообщениями */
#define _FOS_SEND              1
#define _FOS_RECEIVE           2
#define _FOS_REPLY             3
#define _FOS_FORWARD           4

/*
  следующие функции целесообразнее разместить
  в системных вызовах -- очень существенно сказавается
  на производительности
*/
#define _FOS_MASK_INTERRUPT    5
#define _FOS_UNMASK_INTERRUPT  6
#define _FOS_SCHED_YIELD       7
#define _FOS_UPTIME            8
#define _FOS_ALARM             9
#define _FOS_MYTID             10  /* позволяет потоку узнать свой Thread ID */

#define _MSG_SENDER_ANY    0
#define _MSG_SENDER_SIGNAL 1

#define PAGE_SIZE 0x1000

#define SYSTID_NAMER   1
#define SYSTID_PROCMAN 2

static inline u32_t sys_call(volatile u32_t cmd, volatile u32_t arg)
{
  u32_t result;
  __asm__ __volatile__ ("int $0x30":"=a"(result):"b"(cmd), "c"(arg));
  return result;
}

static inline u32_t sys_call2(volatile u32_t cmd, volatile u32_t arg1, volatile u32_t arg2)
{
  u32_t result;
  __asm__ __volatile__ ("int $0x30":"=a"(result):"b"(cmd), "c"(arg1), "d"(arg2));
  return result;
}

static inline int sched_yield()
{
  return sys_call(_FOS_SCHED_YIELD, 0);
}

struct message {
  const void * send_buf;
  size_t send_size;

  void * recv_buf;
  size_t recv_size;

  tid_t  tid;

  u32_t  a0;
  u32_t  a1;
  u32_t  a2;
  u32_t  a3;
} __attribute__ ((packed));

asmlinkage res_t send(struct message *msg);
asmlinkage res_t receive(struct message *msg);
asmlinkage res_t reply(struct message *msg);
asmlinkage res_t forward(struct message *msg, tid_t to);

static inline void mask_interrupt(u32_t int_num)
{
  sys_call(_FOS_MASK_INTERRUPT, int_num);
}

static inline void unmask_interrupt(u32_t int_num)
{
  sys_call(_FOS_UNMASK_INTERRUPT, int_num);
}

//asmlinkage tid_t resolve(char *name);

asmlinkage void exit();
asmlinkage u32_t kill(tid_t tid);
asmlinkage tid_t exec(const char * filename);

asmlinkage void * kmemmap(offs_t ptr, size_t size);

#define MEM_FLAG_LOWPAGE 1

asmlinkage void * kmalloc(size_t size, u32_t flags);
asmlinkage int kfree(off_t ptr);

asmlinkage tid_t thread_create(off_t eip);

asmlinkage res_t interrupt_attach(u8_t n);
asmlinkage res_t interrupt_detach(u8_t n);

asmlinkage int resmgr_attach(const char *pathname);

asmlinkage size_t dmesg(char *buf, size_t count);

asmlinkage u32_t uptime();
asmlinkage u32_t alarm(u32_t ticks); /* сообщение придет  через ticks */
asmlinkage u32_t alarm2(u32_t ticks);  /* сообщение придет, когда ticks < uptime() */
asmlinkage tid_t my_tid();

/* xchg взят из linux-2.6.17 */

struct __xchg_dummy { unsigned long a[4]; };
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

#endif