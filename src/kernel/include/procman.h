/*
  kernel/include/procman.h
  Copyright (C) 2005-2007 Oleg Fedorov
*/

#ifndef _PROCMAN_H
#define _PROCMAN_H

#include <types.h>
#include <process.h>
#include <list.h>
#include <fs.h>

#define FLAG_TSK_READY 1
#define FLAG_TSK_KERN  2
#define FLAG_TSK_TERM  4
#define FLAG_TSK_SEND  8
#define FLAG_TSK_RECV  0x10

#define PROCMAN_CMD_EXEC             (BASE_CMD_N + 0)
#define PROCMAN_CMD_KILL             (BASE_CMD_N + 1)
#define PROCMAN_CMD_EXIT             (BASE_CMD_N + 2)
#define PROCMAN_CMD_MEM_ALLOC        (BASE_CMD_N + 3)
#define PROCMAN_CMD_MEM_MAP          (BASE_CMD_N + 4)
#define PROCMAN_CMD_MEM_FREE         (BASE_CMD_N + 5)
#define PROCMAN_CMD_CREATE_THREAD    (BASE_CMD_N + 6)
#define PROCMAN_CMD_INTERRUPT_ATTACH (BASE_CMD_N + 7)
#define PROCMAN_CMD_INTERRUPT_DETACH (BASE_CMD_N + 8)
#define PROCMAN_CMD_DMESG            (BASE_CMD_N + 9)

class TProcMan {
private:
  List<Thread *> *threadlist;

public:
  TProcMan();

  void reg_thread(register Thread *thread);
  void unreg_thread(register List<Thread *> *thread);

  u32_t exec(register void *image, const string name);
  void scheduler();
  res_t kill(register tid_t tid);
  Thread *get_thread_by_tid(register tid_t tid);
  u32_t curr_proc;
  Thread *current_thread;
};

void kill(tid_t tid);

static inline tid_t TID(Thread *thread)
{
  return (tid_t) thread;
}

static inline Thread * THREAD(tid_t tid)
{
  return (Thread *) tid;
}


#endif
