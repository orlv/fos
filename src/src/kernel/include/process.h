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
  unsigned long send_size;
  void *recv_buf;
  unsigned long recv_size;
  unsigned long pid;
} __attribute__ ((packed));

typedef struct task_mem_block_t {
  offs_t vptr; /* на какой адрес в памяти процесса смонтировано */
  offs_t pptr; /* физический адрес начала блока */
  size_t size; /* размер блока */
};

class TProcess {
private:
  off_t stack_pl0;
  u32_t LoadELF(register void *image);
  u32_t *PageDir; /* каталог страниц */

  List *UsedMem;
  List *FreeMem;

  void mem_init();

  u32_t *CreatePageDir();
  
 public:
  TProcess(register u16_t flags, register void *image, register u32_t *PageDir);
  ~TProcess();
  void run();

  struct TSS *tss;
  gdt_entry descr;
  u8_t flags;
  u32_t alarm;

  void set_stack_pl0();
  void set_tss(register off_t eip, register u32_t *PageDir);
  void kprocess_set_tss(register off_t eip, register u32_t *PageDir);
  List *msg;
  //  void AddMsg(struct message *msg);
  u32_t pid;

  u32_t mount_page(register u32_t phys_page, register u32_t log_page);
  u32_t umount_page(register u32_t log_page);

  void *mem_alloc(register size_t size);
  void *mem_alloc(register offs_t ph_ptr, register size_t size);
  void *mem_alloc(register void *ptr, register size_t size, register void *ph_ptr);
  void mem_free(register void *ptr);
};

struct kmessage {
  void *send_buf;
  size_t send_size;
  void *recv_buf;
  size_t recv_size;
  pid_t pid;
  TProcess *process;
};

#endif
