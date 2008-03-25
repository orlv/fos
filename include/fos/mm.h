/*
    fos/mm.h
    Copyright (C) 2005-2006 Oleg Fedorov
*/

#ifndef _FOS_MM_H
#define _FOS_MM_H

#include <types.h>
#include <c++/list.h>

/*
——————————————————————————————————————————————————————————————————————————————+
                               Карта памяти (i386)                            |
—————————————————+——————————————————————————————+—————————————————————————————+
                 |         Содержимое           |  Адрес начала блока памяти  |
—————————————————+——————————————————————————————+—————————————————————————————+
 Разделяемая     |  BIOS, etc                   | 0                           |
 память.         +——————————————————————————————+—————————————————————————————+
 Присутствует    |  Kernel                      | KERNEL_MEM_BASE (=1Mb)      |
 в каждом        +——————————————————————————————+                             |
 адресном        |  Modules                     |                             |
 пространстве    +——————————————————————————————+—————————————————————————————+
                 |  Free Memory                 | freemem_start_DMA16         |
                 +——————————————————————————————+—————————————————————————————+
                 |  Kernel Heap                 | 16Mb (DMA16)                |
                 +——————————————————————————————+—————————————————————————————+
                 |  Free Kernel Memory          | freemem_start               |
                 +——————————————————————————————+—————————————————————————————+
                 |  Kernel Pagetables           | KERNEL_MEM_LIMIT -          |
		 | (32 таблицы при              | (KERNEL_MEM_LIMIT/PAGE_SIZE)|
		 |   KERNEL_MEM_LIMIT = 128 Мб) |                             |
—————————————————+——————————————————————————————|—————————————————————————————+
                 |  User Memory                 | KERNEL_MEM_LIMIT            | 
                 |                              | или USER_MEM_BASE 	      |	  
—————————————————+——————————————————————————————+—————————————————————————————+
                                                  SYSTEM_MEM_TOP              |
						  или PROCESS_MEM_LIMIT       |
						  (0xfffffff+1)               |
——————————————————————————————————————————————————————————————————————————————+  		  
*/

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
#define DMA16_MEM_SIZE           0x1000000 /* 16 Мб  */

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
  size_t size; /* размер блока */
};

void * realloc(register void *ptr, register size_t size);
void * heap_create_reserved_block();

/* Mapping flags */
#define MAP_FIXED       0x10            /* Interpret addr exactly */
#define MAP_DMA16       0x40              /* allocate memory from DMA16 area or return error */

#ifdef iKERNEL

class VMM {
 private:
  List<memblock *> * volatile FreeMem;
  off_t alloc_free_area(register size_t &lenght);
  off_t cut_free_area(register off_t start, register size_t &lenght);

 public:
  VMM(offs_t base, size_t size);
  ~VMM();

  off_t mem_base;
  size_t mem_size;
  class Pager *pager;

  void *find_free_area(register size_t lenght);
  /* если start=0 -- ищем с find_free_area() память и мапим */
  void *mmap(register off_t start, register size_t lenght, register int flags, off_t from_start, VMM *vm_from);

  /* отсоединить страницы от данного адресного пространства
     (если станицы смонтированы также в другом месте - они там останутся) */
  int munmap(register off_t start, register size_t lenght);
};

void * kmalloc(register size_t size);
void  kfree(register void *ptr, register size_t size);

void init_memory();

#endif

#endif
