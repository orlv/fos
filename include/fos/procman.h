/*
  include/fos/procman.h
  Copyright (C) 2007 Oleg Fedorov
 */

#ifndef _FOS_PROCMAN_H
#define _FOS_PROCMAN_H

#include <types.h>
#include <fos/fs.h>

#define PROCMAN_CMD_EXEC             (BASE_CMD_N + 0)
#define PROCMAN_CMD_KILL             (BASE_CMD_N + 1)
#define PROCMAN_CMD_EXIT             (BASE_CMD_N + 2)
#define PROCMAN_CMD_THREAD_EXIT      (BASE_CMD_N + 3)
#define PROCMAN_CMD_CREATE_THREAD    (BASE_CMD_N + 4)
#define PROCMAN_CMD_INTERRUPT_ATTACH (BASE_CMD_N + 5)
#define PROCMAN_CMD_INTERRUPT_DETACH (BASE_CMD_N + 6)
#define PROCMAN_CMD_DMESG            (BASE_CMD_N + 7)

#define MM_CMD_MEM_ALLOC        (BASE_CMD_N + 0)
#define MM_CMD_MEM_MAP          (BASE_CMD_N + 1)
#define MM_CMD_MEM_FREE         (BASE_CMD_N + 2)

#ifdef iKERNEL

#include <fos/process.h>
#include <c++/list.h>

#define FLAG_TSK_READY        0x01
#define FLAG_TSK_KERN         0x02
#define FLAG_TSK_TERM         0x04 /* завершить все потоки в данном адресном пространстве */
#define FLAG_TSK_EXIT_THREAD  0x08 /* завершить только один поток */
#define FLAG_TSK_SEND         0x10
#define FLAG_TSK_RECV         0x20
#define FLAG_TSK_SYSCALL      0x40 /* поток выполняет системный вызов */

class TProcMan {
private:
  List<Thread *> *threadlist;

public:
  TProcMan();

  void reg_thread(register Thread *thread);
  void unreg_thread(register List<Thread *> *thread);

  u32_t exec(register void *image, const string name);
  void scheduler();
  List<Thread *> *do_kill(List<Thread *> *thread);
  res_t kill(register tid_t tid, u16_t flag);
  Thread *get_thread_by_tid(register tid_t tid);
  u32_t curr_proc;
  Thread *current_thread;
};

static inline tid_t TID(Thread *thread)
{
  return (tid_t) thread;
}

static inline Thread * THREAD(tid_t tid)
{
  return (Thread *) tid;
}

#endif /* iKERNEL */


asmlinkage u32_t kill(tid_t tid);
asmlinkage tid_t exec(const char * filename);

asmlinkage void * kmemmap(offs_t ptr, size_t size);

#define MEM_FLAG_LOWPAGE 1

asmlinkage void * kmalloc(size_t size, u32_t flags);
asmlinkage int kfree(off_t ptr);

asmlinkage tid_t thread_create(off_t eip);

asmlinkage int resmgr_attach(const char *pathname);

asmlinkage size_t dmesg(char *buf, size_t count);

#endif
