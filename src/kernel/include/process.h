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

/*
  Карта памяти:

               / ------------------------------ 0
               |     BIOS, etc
               |  ----------------------------- KERNEL_MEM_BASE (1Mb)
               |     Kernel
               |     Modules
               |  ----------------------------- low_freemem_start
               |
Разделяемая    |     Free Memory  
память,        /  
присутствует  <    ---------------------------- 16Mb
в каждом       \ 
адресном       |    Kernel Heap
пространстве   |
               |  ----------------------------- freemem_start
               |
               |    Free Memory
               | 
               |  -----------------------------
               |     Kernel Pagetables (32)
               \  ----------------------------- USER_PAGEDIR_DATA
                     User Pagetables (33-1024)
                  ----------------------------- KERNEL_MEM_LIMIT | USER_MEM_BASE


		  
                     User Memory


		     
                  ----------------------------- SYSTEM_MEM_TOP | PROCESS_MEM_LIMIT

 */

#define STACK_SIZE 4096
#define SYSTEM_MEM_TOP 0xffffffff
#define SYSTEM_PAGES_MAX 100000

#define USER_MEM_BASE 0x8000000 /* 128 мегабайт */
#define USER_MEM_SIZE (SYSTEM_MEM_TOP - USER_MEM_BASE)
#define USER_MEM_LIMIT SYSTEM_MEM_TOP

#define KERNEL_MEM_BASE 0x100000 /* 1 мегабайт */
#define KERNEL_MEM_SIZE (USER_MEM_BASE - KERNEL_MEM_BASE)
#define KERNEL_MEM_LIMIT (KERNEL_MEM_BASE + KERNEL_MEM_SIZE)

#define USER_PAGETABLE_DATA_SIZE (((SYSTEM_PAGES_MAX-(KERNEL_MEM_LIMIT/PAGE_SIZE))/1024)*4096) /* 3,875 Mb */
#define USER_PAGETABLE_DATA (KERNEL_MEM_LIMIT-USER_PAGETABLE_DATA_SIZE)


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
  List *UsedMem;
  List *FreeMem;
  u32_t *CreatePageDir();

  void map_pages(register u32_t *phys_pages, register u32_t log_page, register size_t n);
  void umap_pages(register u32_t *log_pages, register size_t n);

  void *mem_alloc(register u32_t *phys_pages, register size_t pages_cnt);
  void *do_mmap(register u32_t *phys_pages, register void *log_address, register size_t pages_cnt);

 public:
  TProcess();
  ~TProcess();
  void run();

  u32_t *PageDir; /* каталог страниц */
  List *threads;

  void mem_init(offs_t base, size_t size);
  
  Thread *thread_create(off_t eip, u16_t flags);

  u32_t mount_page(register u32_t phys_page, register u32_t log_page);
  u32_t umount_page(register u32_t log_page);

  void *mem_alloc(register size_t size);
  void *mem_alloc_phys(register u32_t phys_address, register size_t size);
  void *mmap(register size_t size, register void *log_address);
  void *mmap(register void *phys_address, register void *log_address, register size_t size);
  void mem_free(register void *ptr);
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
