/*
  include/fos/page.h
  Copyright (C) 2007 Oleg Fedorov
 */

#ifndef _FOS_PAGE_H
#define _FOS_PAGE_H

#include <types.h>

#ifdef iKERNEL
#include <c++/atomic.h>
#endif

#define PAGE_SIZE 0x1000

asmlinkage void * kmemmap(offs_t ptr, size_t size);

#define MEM_FLAG_LOWPAGE 1

asmlinkage void * kmalloc(size_t size, u32_t flags);
asmlinkage int kfree(off_t ptr);

#ifdef iKERNEL
struct page {
  atomic_t mapcount;          /* общее количество использований страницы */
  volatile u32_t kernel_map;  /* на какой логический адрес в области ядра смонтировано (если смонтировано) */
};

static inline u32_t PAGE(u32_t address)
{
  return address/PAGE_SIZE;
}
#endif

#endif
