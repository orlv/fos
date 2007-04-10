/*
        kernel/include/tasks.h
        Copyright (C) 2005-2006 Oleg Fedorov
*/

#ifndef _TASKS_H
#define _TASKS_H

#include <types.h>
#include <tss.h>
#include <list.h>
#include <dt.h>

#define MAX_PROCS 250
#define STACK_SIZE 4096

#define FLAG_TSK_READY 1
#define FLAG_TSK_LIGHT 2
#define FLAG_TSK_TERM  4
#define FLAG_TSK_SEND  8
#define FLAG_TSK_RECV  0x10

#define PROCESS_MEM_BASE 0x3200000
#define PROCESS_MEM_LIMIT (0xffffffff - 0x3200000)

typedef u32_t pid_t;

asmlinkage u32_t terminate(pid_t pid);

struct message {
  void *send_buf;
  unsigned long send_size;
  void *recv_buf;
  unsigned long recv_size;
  unsigned long pid;
} __attribute__ ((packed));

typedef struct task_mem_block_t {
  offs_t ptr;
  size_t size;
};

class TProcess {
private:
  off_t stack_pl0;
  u32_t LoadELF(void *image);
  u32_t *PageDir;		/* каталог страниц */

  List *UsedMem;
  List *FreeMem;

  void mem_init();

public:
   TProcess(void *image, u16_t flags, u32_t * PageDir, u32_t crutch);
  void run();

  struct TSS *tss;
  dt_t descr;
  u8_t flags;
  u32_t alarm;

  void set_stack_pl0();
  void set_tss(off_t eip, u32_t cr3, u32_t crunch);
  List *msg;
  //  void AddMsg(struct message *msg);
  u32_t pid;

  u32_t mount_page(u32_t phys_page, u32_t log_page);
  u32_t umount_page(u32_t log_page);

  void *mem_alloc(size_t size);
  void *mem_alloc(offs_t ph_ptr, size_t size);
  void *mem_alloc(void *ptr, size_t size, void *ph_ptr);
  void mem_free(void *ptr);
};

class TProcMan {
private:
  List * proclist;
  u32_t *kPageDir;
  u32_t *CreatePageDir();

public:
   TProcMan();

  void add(TProcess * Process);
  void del(List * proc);

  u32_t exec(void *image);

  void scheduler();

  res_t stop(pid_t pid);

  TProcess *NewLightProc(off_t eip, u16_t flags);
  TProcess *get_process_by_pid(u32_t pid);

  u32_t curr_proc;
};

void start_sched();
u32_t create_pagedir();

#endif
