/*
    kernel/include/mm.h
    Copyright (C) 2005-2006 Oleg Fedorov
*/

#ifndef _MEMORY_H
#define _MEMORY_H

#include <types.h>
#include <atomic.h>

/* размер gdt - 64 килобайта */
#define GDT_DESCR 8192

#define IDT_DESCR 256

#define PAGE_SIZE 0x1000

#define KERNEL_CODE 0x08
#define KERNEL_DATA 0x10
#define USER_CODE   0x1b
#define USER_DATA   0x23

#define MM_MINALLOC PAGE_SIZE	/* размер выделяемой единицы */

struct page {
  atomic_t mapcount;
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

void put_page(u32_t page);
u32_t get_page();
void * kmalloc(register size_t size);
void  kmfree(register void *ptr, register size_t size);

u32_t map_page(register u32_t phys_page, register u32_t log_page, register u32_t * pagedir, register u8_t c3wp);
u32_t umap_page(register u32_t log_page, register u32_t * pagedir);

void init_memory();
void enable_paging(u32_t * pagedir);

#endif
