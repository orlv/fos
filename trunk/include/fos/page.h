/*
  include/fos/page.h
  Copyright (C) 2007 Oleg Fedorov
 */

#ifndef _FOS_PAGE_H
#define _FOS_PAGE_H

#include <types.h>

#define PAGE_SIZE 0x1000

asmlinkage off_t getpagephysaddr(off_t pageaddr); /* возвращает физический адрес страницы */

//#define MEM_FLAG_LOWPAGE 1

#ifdef iKERNEL
struct page {
  volatile size_t mapcount;          /* общее количество использований страницы */
  volatile u32_t kernel_map;  /* на какой логический адрес в области ядра смонтировано (если смонтировано) */
};

static inline u32_t PAGE(u32_t address)
{
  return address/PAGE_SIZE;
}
#endif

#endif
