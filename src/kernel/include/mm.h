/*
    kernel/include/mm.h
    Copyright (C) 2005-2006 Oleg Fedorov
*/

#ifndef _MEMORY_H
#define _MEMORY_H

#include <types.h>
#include <atomic.h>
#include <list.h>

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
               |     Kernel Pagetables (32)
               \ ------------------------------ KERNEL_MEM_LIMIT | USER_MEM_BASE


		  
                     User Memory


		     
                  ----------------------------- SYSTEM_MEM_TOP | PROCESS_MEM_LIMIT

 */


/* размер gdt - 64 килобайта */
#define GDT_DESCR 8192

#define IDT_DESCR 256

#define PAGE_SIZE 0x1000

#define KERNEL_CODE_SEGMENT 0x08
#define KERNEL_DATA_SEGMENT 0x10
#define USER_CODE_SEGMENT   0x1b
#define USER_DATA_SEGMENT   0x23

#define MM_MINALLOC PAGE_SIZE	/* размер выделяемой единицы */

#define STACK_SIZE 4096
#define SYSTEM_MEM_TOP 0xffffffff
#define SYSTEM_PAGES_MAX 100000

#define USER_MEM_BASE 0x8000000 /* 128 мегабайт */
#define USER_MEM_SIZE (SYSTEM_MEM_TOP - USER_MEM_BASE)
#define USER_MEM_LIMIT SYSTEM_MEM_TOP

#define KERNEL_MEM_BASE 0
#define KERNEL_MEM_SIZE (USER_MEM_BASE - KERNEL_MEM_BASE)
#define KERNEL_MEM_LIMIT (KERNEL_MEM_BASE + KERNEL_MEM_SIZE)

//#define USER_PAGETABLE_DATA_SIZE (((SYSTEM_PAGES_MAX-(KERNEL_MEM_LIMIT/PAGE_SIZE))/1024)*4096) /* 3,875 Mb */
//#define USER_PAGETABLE_DATA (KERNEL_MEM_LIMIT-USER_PAGETABLE_DATA_SIZE)


struct page {
  atomic_t mapcount; /* общее количество использований страницы */
  volatile u32_t kernel_map;  /* на какой логический адрес в области ядра смонтировано (если смонтировано) */
};

struct HeapMemBlock {
  HeapMemBlock *ptr;
  unsigned int size;
};

struct memstack {
  memstack *next;
  u32_t n;
};

static inline u32_t PAGE(u32_t address)
{
  return address/PAGE_SIZE;
}

#define MMU_PAGE_PRESENT         1
#define MMU_PAGE_WRITE_ACCESS    2
#define MMU_PAGE_USER_ACCESSABLE 4

struct memblock {
  offs_t vptr; /* на какой адрес в памяти процесса смонтировано */
  u32_t *phys_pages; /* массив номеров физических страниц, использованных в блоке */
  size_t size; /* размер блока */
};

void *realloc(register void *ptr, register size_t size);

class Memory {
 private:
  List<memblock *> * volatile UsedMem;
  List<memblock *> * volatile FreeMem;
  u16_t flags;

  void map_pages(register u32_t *phys_pages, register u32_t log_page, register size_t n);
  void umap_pages(register u32_t log_page, register size_t n);

  void *mem_alloc(register u32_t *phys_pages, register size_t pages_cnt);
  void *do_mmap(register u32_t *phys_pages, register void *log_address, register size_t pages_cnt);

 public:
  Memory(offs_t base, size_t size, u16_t flags);

  u32_t *pagedir; /* каталог страниц */

  u32_t mount_page(register u32_t phys_page, register u32_t log_page);
  /* если указанная страница более нигде не используется - она добавляется в пул свободных страниц */
  u32_t umount_page(register u32_t log_page);

  void *mem_alloc(register size_t size);
  void *mem_alloc_phys(register u32_t phys_address, register size_t size);
  void *mmap(register size_t size, register void *log_address);
  void *mmap(register void *phys_address, register void *log_address, register size_t size);
  void *kmmap(register void *kmem_address, register void *log_address, register size_t size);
  void mem_free(register void *ptr);
  void dump_used();
  void dump_free();
};

void put_page(u32_t page);
u32_t get_page();
void * kmalloc(register size_t size);
void  kfree(register void *ptr);

u32_t map_page(register u32_t phys_page, register u32_t log_page, register u32_t * pagedir, register u16_t flags);
u32_t umap_page(register u32_t log_page, register u32_t * pagedir);

void init_memory();
void enable_paging(u32_t * pagedir);

#endif
