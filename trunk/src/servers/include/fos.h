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

static inline void sys_call(volatile u32_t arg1, volatile u32_t arg2)
{
  __asm__ __volatile__ ("int $0x30"::"b" (arg1), "c"(arg2));
}

struct message {
  void *send_buf;
  size_t send_size;
  void *recv_buf;
  size_t recv_size;
  tid_t tid;
} __attribute__ ((packed));

asmlinkage void reply(struct message *msg);
asmlinkage void send(struct message *msg);
asmlinkage void receive(struct message *msg);

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

asmlinkage int resmgr_attach(const char *pathname);

asmlinkage size_t dmesg(char *buf, size_t count);

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
