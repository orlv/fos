/*
 * kernel/memory/mmu.cpp
 * Copyright (C) 2005-2007 Oleg Fedorov
 */

#include <system.h>
#include <paging.h>
#include <stdio.h>
#include <mm.h>
#include <hal.h>

static inline u32_t * pagetable_addr(u32_t number, u32_t *pagedir)
{
  return (u32_t *)((pagedir)[number/1024] & 0xfffff000);
}

void enable_paging(u32_t * pagedir)
{
  /*
   * Поместим адрес каталога страниц в CR3. 
   * Включим страничную адресацию.
   */

  __asm__ __volatile__("movl %%eax, %%cr3\n" \
      "movl %%cr0, %%eax\n" \
      "orl $0x80000000, %%eax\n" \
      "movl %%eax, %%cr0": :"a"(pagedir));
}

u32_t umap_page(register u32_t log_page, register u32_t * pagedir)
{
  u32_t *pagetable;
  u32_t page;
  pagetable = pagetable_addr(log_page, pagedir);
  /* Узнаём физический адрес страницы */
  page = pagetable[log_page & 0x3ff] & 0xfffff000;
  if(page){
    put_page(PAGE(page));
  }
  pagetable[log_page & 0x3ff] = 0;
  return (page);
}

u32_t map_page(register u32_t phys_page, register u32_t log_page, register u32_t * pagedir, register u16_t flags)
{
  alloc_page(phys_page);

  /*
   * Узнаем физический адрес таблицы, в которой будет расположена страница
   */
  u32_t *pagetable = pagetable_addr(log_page, pagedir);
  /* Если pagetable не существует - создаём её */
  if (!pagetable) {

    //while(1) asm("incb 0xb8000+156\n" "movb $0x2f,0xb8000+157 ");
    pagetable = (u32_t *) kmalloc(PAGE_SIZE); /* таблица страниц должна быть доступна из любого адресного
				       пространства - поэтому используем kmalloc() */
    pagedir[log_page / 0x400] = (u32_t) pagetable | flags;
  }
  pagetable[log_page & 0x3ff] = (phys_page * 0x1000) | flags;
  return ((unsigned long)pagetable);
}
