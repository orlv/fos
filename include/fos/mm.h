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
               | ------------------------------ freemem_start_DMA16
               |
Разделяемая    |     Free Memory  
память,        /  
присутствует  <  ------------------------------ 16Mb (DMA16)
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
#define DMA16_MEM_SIZE           0x1000000 /* 16 Мб  */


#define HEAP_RESERVED_BLOCK_SIZE 0x1000

//#define USER_PAGETABLE_DATA_SIZE (((SYSTEM_PAGES_MAX-(KERNEL_MEM_LIMIT/PAGE_SIZE))/1024)*4096) /* 3,875 Mb */
//#define USER_PAGETABLE_DATA (KERNEL_MEM_LIMIT-USER_PAGETABLE_DATA_SIZE)

struct HeapMemBlock{
  union {
    struct{
      struct HeapMemBlock *next;
      size_t  size;
    };
    u8_t align[16];
  };
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
  size_t size; /* размер блока */
};

void * realloc(register void *ptr, register size_t size);
void * heap_create_reserved_block();


/* Protection bits */
//#define PROT_READ  0x1 /* page can be read */
//#define PROT_WRITE 0x2 /* page can be written */
//#define PROT_EXEC  0x4 /* page can be executed */
//#define PROT_SEM   0x8 /* page may be used for atomic ops */
//#define PROT_NONE  0x0 /* page can not be accessed */

/* Mapping flags */
//#define MAP_SHARED      0x01            /* Share changes */
//#define MAP_PRIVATE     0x02            /* Changes are private */
//#define MAP_TYPE        0x0f            /* Mask for type of mapping */
#define MAP_FIXED       0x10            /* Interpret addr exactly */
//#define MAP_ANONYMOUS   0x20            /* don't use a file */
#define MAP_DMA16       0x40              /* allocate memory from DMA16 area or return error */

#ifdef iKERNEL

class VMM {
 private:
  //List<memblock *> * volatile UsedMem;
  List<memblock *> * volatile FreeMem;
  //u16_t flags;

  //void *mem_alloc(register u32_t *phys_pages, register size_t pages_cnt);
  //  void *do_mmap(register u32_t *phys_pages, register void *log_address, register size_t pages_cnt);
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
