/*
 * fos/mmu.h
 * Copyright (C) 2005-2007 Oleg Fedorov
 */

#ifndef _FOS_MMU_H
#define _FOS_MMU_H

#include <types.h>
#include <fos/page.h>
#include <fos/system.h>

#define MMU_PAGE_PRESENT         1
#define MMU_PAGE_WRITE_ACCESS    2
#define MMU_PAGE_USER_ACCESSABLE 4

#define clear_page(page) memset((void *)(page*PAGE_SIZE), 0, PAGE_SIZE)

#ifdef iKERNEL
static inline u32_t * pagetable_addr(register u32_t n, register u32_t *pagedir)
{
  return (u32_t *)((pagedir)[n/1024] & 0xfffff000);
}

static inline u32_t * phys_addr_from(register u32_t n, register u32_t *pagedir)
{
  u32_t *pagetable = pagetable_addr(n, pagedir);
  pagetable = (u32_t *)(kmem_log_addr(PAGE(OFFSET(pagetable))) * PAGE_SIZE);
  return (u32_t *) (pagetable[n & 0x3ff] & 0xfffff000);
}

/* если страница выдана kmalloc() - возвратит её физический адрес */
u32_t * kmem_phys_addr(u32_t n);

static inline void flush_tlb_single(u32_t addr)
{
  __asm__ __volatile__("invlpg (%0)"::"r" (addr): "memory");
}

u32_t map_page(register u32_t phys_page, register u32_t log_page, register u32_t * pagedir, register u16_t flags);

/* если указанная страница более нигде не используется - она добавляется в пул свободных страниц */
u32_t umap_page(register u32_t log_page, register u32_t * pagedir);
#endif

#endif
