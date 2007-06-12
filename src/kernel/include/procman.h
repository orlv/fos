/*
  kernel/include/procman.h
  Copyright (C) 2005-2007 Oleg Fedorov
*/

#ifndef _PROCMAN_H
#define _PROCMAN_H

#include <types.h>
#include <process.h>
#include <list.h>

#define MAX_PROCS 250

#define FLAG_TSK_READY 1
#define FLAG_TSK_KERN  2
#define FLAG_TSK_TERM  4
#define FLAG_TSK_SEND  8
#define FLAG_TSK_RECV  0x10

struct procman_message {
  u32_t cmd;
  union {
    char buf[252];
    u32_t pid;
    u32_t value;
    struct {
      u32_t a1;
      u32_t a2;
    }val;
  }arg;
} __attribute__ ((packed));

#define PROCMAN_CMD_EXEC             0
#define PROCMAN_CMD_KILL             1
#define PROCMAN_CMD_EXIT             2
#define PROCMAN_CMD_MEM_ALLOC        3
#define PROCMAN_CMD_MEM_MAP          4
#define PROCMAN_CMD_CREATE_THREAD    5
#define PROCMAN_CMD_INTERRUPT_ATTACH 6
#define PROCMAN_CMD_INTERRUPT_DETACH 7

class TProcMan {
private:
  List * proclist;
  //volatile u32_t top_pid;

public:
  TProcMan();

  void add(register Thread * thread);
  void del(register List * proc);

  u32_t exec(register void *image);

  void scheduler();

  /* уничтожить процесс */
  res_t kill(register tid_t tid);

  TProcess *kprocess(register off_t eip, register u16_t flags);
  Thread *get_thread_by_tid(register tid_t tid);

  u32_t curr_proc;

  Thread *CurrentThread;

  /* 
  inline u32_t get_pid()
  {
    top_pid++;
    return top_pid-1;;
    }*/

  u32_t *kPageDir;
};

void kill(tid_t tid);

#endif