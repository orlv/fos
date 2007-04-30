/*
    kernel/include/mm.h
    Copyright (C) 2005-2006 Oleg Fedorov
*/

#ifndef _MEMORY_H
#define _MEMORY_H

#include <types.h>

//#define KRNL_STACK_BASE 0xA0000
//#define KRNL_STACK_SIZE 0x

//#define GDT_BASE (KRNL_STACK_BASE + KRNL_STACK_SIZE)
/* размер gdt - 64 килобайта */
#define GDT_DESCR 8192
//#define GDT_SIZE (8 * GDT_DESCR)

//#define IDT_BASE (GDT_BASE + GDT_SIZE)
#define IDT_DESCR 256
//#define IDT_SIZE (8 * IDT_DESCR)

#define PAGE_SIZE 0x1000

#define KERNEL_CODE 0x08
#define KERNEL_DATA 0x10
#define USER_CODE   0x1b
#define USER_DATA   0x23

#define MM_MINALLOC PAGE_SIZE	/* размер выделяемой единицы */

typedef struct HeapMemBlock {
  HeapMemBlock *ptr;
  unsigned int size;
};

void *kmalloc(size_t size);
void kmfree(void *ptr, size_t size);

void init_alloc();

class Heap {
private:
  HeapMemBlock * free_ptr;
  HeapMemBlock kmem_block;

  HeapMemBlock *morecore(unsigned int nu);

public:
   Heap();
  ~Heap();

  void *malloc(size_t size);
  void free(void *ptr);
  void *realloc(void *ptr, size_t size);
};

void init_memory();

#endif
