/*
    fos/mm.h
    Copyright (C) 2005-2006 Oleg Fedorov
*/

#ifndef _FOS_MM_H
#define _FOS_MM_H

#include <types.h>
#include <c++/atomic.h>
#include <c++/list.h>

/*
  Карта памяти:

               / ------------------------------ 0
               |     BIOS, etc
               | ------------------------------ KERNEL_MEM_BASE (1Mb)
               |     Kernel
               |     Modules
               | ------------------------------ low_freemem_start
               |
Разделяемая    |     Free Memory  
память,        /  
присутствует  <  ------------------------------ 16Mb
в каждом       \ 
адресном       |    Kernel Heap
пространстве   |
               | ------------------------------ freemem_start
               |
               |    Free Memory
               | 
               | ------------------------------
               |     Kernel Pagetables (32 при KERNEL_MEM_LIMIT = 128 Мб)
               \ ------------------------------ KERNEL_MEM_LIMIT | USER_MEM_BASE


		  
                     User Memory


		     
                  ----------------------------- SYSTEM_MEM_TOP | PROCESS_MEM_LIMIT

 */


/* размер gdt - 64 килобайта */
#define GDT_DESCR 8192

#define IDT_DESCR 256

#define KERNEL_CODE_SEGMENT 0x08
#define KERNEL_DATA_SEGMENT 0x10
#define USER_CODE_SEGMENT   0x1b
#define USER_DATA_SEGMENT   0x23

#define MM_MINALLOC      PAGE_SIZE /* размер выделяемой единицы */

#define STACK_SIZE       4096
#define SYSTEM_MEM_TOP   0xffffffff
#define SYSTEM_PAGES_MAX 100000

#define USER_MEM_BASE    0x8000000 /* 128 мегабайт */
#define USER_MEM_SIZE    (SYSTEM_MEM_TOP - USER_MEM_BASE)
#define USER_MEM_LIMIT   SYSTEM_MEM_TOP

#define KERNEL_MEM_BASE  0
#define KERNEL_MEM_SIZE  (USER_MEM_BASE - KERNEL_MEM_BASE)
#define KERNEL_MEM_LIMIT (KERNEL_MEM_BASE + KERNEL_MEM_SIZE)

#define MIN_FREE_MEMORY       0x0100000 /* 64 Кб  */
#define KERNEL_MIN_HEAP_SIZE  0x0050000 /* 64 Кб  */
#define LOWMEM_SIZE           0x1000000 /* 16 Мб  */


#define HEAP_RESERVED_BLOCK_SIZE 0x1000

//#define USER_PAGETABLE_DATA_SIZE (((SYSTEM_PAGES_MAX-(KERNEL_MEM_LIMIT/PAGE_SIZE))/1024)*4096) /* 3,875 Mb */
//#define USER_PAGETABLE_DATA (KERNEL_MEM_LIMIT-USER_PAGETABLE_DATA_SIZE)

struct HeapMemBlock {
  HeapMemBlock *next;
  size_t size;
};

struct memstack {
  memstack *next;
  u32_t n;
};

static inline u32_t OFFSET(void *ptr)
{
  return (u32_t) ptr;
}

static inline u32_t OFFSET(const void *ptr)
{
  return (u32_t) ptr;
}

static inline void * ADDRESS(u32_t offset)
{
  return (void *) offset;
}

struct memblock {
  offs_t vptr; /* на какой адрес в памяти процесса смонтировано */
  //u32_t *phys_pages; /* массив номеров физических страниц, использованных в блоке */
  size_t size; /* размер блока */
};

void * realloc(register void *ptr, register size_t size);
void * heap_create_reserved_block();

class Memory {
 private:
  //List<memblock *> * volatile UsedMem;
  List<memblock *> * volatile FreeMem;
  //u16_t flags;

  void *mem_alloc(register u32_t *phys_pages, register size_t pages_cnt);
  void *do_mmap(register u32_t *phys_pages, register void *log_address, register size_t pages_cnt);

 public:
  Memory(offs_t base, size_t size);
  ~Memory();

  off_t mem_base;
  class Pager *pager;
  
  void *mem_alloc(register size_t size);
  void *mem_alloc_phys(register u32_t phys_address, register size_t size);
  void *kmem_alloc(register void *kmem_address, register size_t size);
  void *mmap(register size_t size, register void *log_address);
  void *mmap(register void *phys_address, register void *log_address, register size_t size);
  void *kmmap(register void *kmem_address, register void *log_address, register size_t size);
  void mem_free(register void *ptr, register size_t size);
};

void * kmalloc(register size_t size);
void  kfree(register void *ptr, register size_t size);

void init_memory();

#endif
