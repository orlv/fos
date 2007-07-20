/*
 * kernel/include/mmu.h
 * Copyright (C) 2005-2007 Oleg Fedorov
 */

#ifndef __MMU_H
#define __MMU_H

#include <types.h>
#include <hal.h>
#include <string.h>
#include <stdio.h>

void setup_paging();

#define clear_page(page) memset((void *)(page), 0, PAGE_SIZE)

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

static inline void flush_tlb_single(u32_t addr)
{
  __asm__ __volatile__("invlpg (%0)"::"r" (addr): "memory");
}

static inline u32_t check_page(register u32_t log_page, register u32_t * pagedir)
{
  __mt_disable();
  u32_t *pagetable = (u32_t *)(kmem_log_addr(PAGE(OFFSET(pagetable_addr(log_page, pagedir)))) * PAGE_SIZE);

  if(!pagetable) {
    __mt_enable();
    return 0;
  }

  u32_t page = pagetable[log_page & 0x3ff] & 0xfffff000;

  __mt_enable();
  return (page);
}

static inline u32_t check_pages(register u32_t log_page, register u32_t * pagedir, register size_t count)
{
  while(count) {
    if(!check_page(log_page, pagedir))
      return 0;
    count--;
    log_page++;
  }
  return 1;
}

#endif
