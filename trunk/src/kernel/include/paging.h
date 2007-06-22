/*
 * kernel/include/paging.h
 * Copyright (C) 2005-2007 Oleg Fedorov
 */

#ifndef __PAGING_H
#define __PAGING_H

#include <types.h>
#include <hal.h>

void setup_paging();

static inline u32_t * pagetable_addr(u32_t n, u32_t *pagedir)
{
  return (u32_t *)((pagedir)[n/1024] & 0xfffff000);
}

/* если страница выдана kmalloc() - возвратит её физический адрес */
static inline u32_t * kmem_phys_addr(u32_t n)
{
  u32_t *pagetable = pagetable_addr(n, hal->kmem->pagedir);
  return (u32_t *) (pagetable[n & 0x3ff] & 0xfffff000);
}

#endif
