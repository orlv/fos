/*
        kernel/include/process.h
        Copyright (C) 2005-2007 Oleg Fedorov
*/

#ifndef _PROCESS_H
#define _PROCESS_H

#include <types.h>
#include <tss.h>
#include <list.h>
#include <gdt.h>

//#define MAX_PROCS 250
#define STACK_SIZE 4096

//#define FLAG_TSK_READY 1
//#define FLAG_TSK_LIGHT 2
//#define FLAG_TSK_TERM  4
//#define FLAG_TSK_SEND  8
//#define FLAG_TSK_RECV  0x10

#define PROCESS_MEM_BASE 0x3200000
#define PROCESS_MEM_LIMIT (0xffffffff - 0x3200000)

//asmlinkage u32_t terminate(pid_t pid);

struct message {
  void *send_buf;
  size_t send_size;
  void *recv_buf;
  size_t recv_size;
  //pid_t pid;
  tid_t tid;
} __attribute__ ((packed));

typedef struct task_mem_block_t {
  offs_t vptr; /* на какой адрес в памяти процесса смонтировано */
  offs_t pptr; /* физический адрес начала блока */
  size_t size; /* размер блока */
};

class Thread{
 private:
  off_t stack_pl0;
  
 public:
  Thread(class TProcess *process, off_t eip, u16_t flags);
  ~Thread();
  void run();

  class TProcess *process;
  struct TSS *tss;
  gdt_entry descr;
  u16_t flags;
  tid_t send_to;
  void set_stack_pl0();
  void set_tss(register off_t eip);
  void kprocess_set_tss(register off_t eip);
  List *new_msg;
  List *recvd_msg;
};

class TProcess {
private:
  u32_t LoadELF(register void *image);
  List *UsedMem;
  List *FreeMem;
  void mem_init();
  u32_t *CreatePageDir();
  
 public:
  TProcess(register u16_t flags, register void *image, register u32_t *PageDir);
  ~TProcess();
  void run();

  u32_t *PageDir; /* каталог страниц */
  List *threads;

  Thread *thread_create(off_t eip, u16_t flags);

  u32_t mount_page(register u32_t phys_page, register u32_t log_page);
  u32_t umount_page(register u32_t log_page);

  void *mem_alloc(register size_t size);
  void *mem_alloc(register offs_t ph_ptr, register size_t size);
  void *mem_alloc(register void *ptr, register size_t size, register void *ph_ptr);
  void mem_free(register void *ptr);
};

#define MESSAGE_ASYNC 1

struct kmessage {
  void * send_buf;
  size_t send_size;
  void * recv_buf;
  size_t recv_size;
  //  TProcess * process;
  Thread * thread;
  u32_t flags;
};

#endif
