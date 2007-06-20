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
#include <mm.h>

struct message {
  void *send_buf;
  size_t send_size;
  void *recv_buf;
  size_t recv_size;
  tid_t tid;
} __attribute__ ((packed));

struct task_mem_block_t {
  offs_t vptr; /* на какой адрес в памяти процесса смонтировано */
  u32_t *phys_pages; /* массив номеров физических страниц, использованных в блоке */
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
  //List *UsedMem;
  //  List *FreeMem;
  //  u32_t *CreatePageDir();

  //  void map_pages(register u32_t *phys_pages, register u32_t log_page, register size_t n);
  //  void umap_pages(register u32_t *log_pages, register size_t n);

  //  void *mem_alloc(register u32_t *phys_pages, register size_t pages_cnt);
  //  void *do_mmap(register u32_t *phys_pages, register void *log_address, register size_t pages_cnt);

 public:
  TProcess();
  ~TProcess();
  void run();

  //  u32_t *PageDir; /* каталог страниц */
  List *threads;
  Memory *memory;

  //void mem_init(offs_t base, size_t size);
  
  Thread *thread_create(off_t eip, u16_t flags);

  //  u32_t mount_page(register u32_t phys_page, register u32_t log_page);
  //  u32_t umount_page(register u32_t log_page);

  //void *mem_alloc(register size_t size);
  //void *mem_alloc_phys(register u32_t phys_address, register size_t size);
  //void *mmap(register size_t size, register void *log_address);
  //  void *mmap(register void *phys_address, register void *log_address, register size_t size);
  //  void mem_free(register void *ptr);
};

#define MESSAGE_ASYNC 1

struct kmessage {
  void * send_buf;
  size_t send_size;
  void * recv_buf;
  size_t recv_size;
  Thread * thread;
  u32_t flags;
};

#endif
