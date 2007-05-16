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
#define FLAG_TSK_LIGHT 2
#define FLAG_TSK_TERM  4
#define FLAG_TSK_SEND  8
#define FLAG_TSK_RECV  0x10

typedef u32_t pid_t;

struct procman_message {
  u32_t cmd;
  u8_t buf[252];
} __attribute__ ((packed));

#define PROCMAN_CMD_EXEC   0
#define PROCMAN_CMD_TERM   1
#define PROCMAN_CMD_PAUSE  3
#define PROCMAN_CMD_RESUME 4

class TProcMan {
private:
  List * proclist;
  u32_t *kPageDir;
  u32_t *CreatePageDir();

  volatile u32_t top_pid;

public:
  TProcMan();

  void add(register TProcess * Process);
  void del(register List * proc);

  u32_t exec(register void *image);

  void scheduler();

  res_t stop(register pid_t pid);

  TProcess *kprocess(register off_t eip, register u16_t flags);
  TProcess *get_process_by_pid(register u32_t pid);

  u32_t curr_proc;

  TProcess *CurrentProcess;

  inline u32_t get_pid()
  {
    top_pid++;
    return top_pid-1;;
  }

};

#endif
