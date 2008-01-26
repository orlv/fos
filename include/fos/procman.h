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

#define MM_CMD_MMAP          (BASE_CMD_N + 0)
#define MM_CMD_MUNMAP        (BASE_CMD_N + 1)

#ifdef iKERNEL

#include <fos/process.h>
#include <c++/list.h>
#include <c++/index.h>

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

  tindex<Thread> *threads;
  tid_t reg_thread(register Thread *thread);
  void unreg_thread(register List<Thread *> *thread);

  tid_t exec(register void *image, const string name,
	     const string args, size_t args_len,
	     const string envp, size_t envp_len);
  void scheduler();
  List<Thread *> *do_kill(List<Thread *> *thread);
  res_t kill(register tid_t tid, u16_t flag);
  u32_t curr_proc;
  Thread *current_thread;
};

/*static inline tid_t TID(Thread *thread)
{
  return thread->tid;
  }*/

#endif /* iKERNEL */

#define MEM_FLAG_LOWPAGE 1

#endif
