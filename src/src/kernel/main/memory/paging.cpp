/*
 * kernel/memory/paging.cpp
 * Copyright (C) 2005-2006 Oleg Fedorov
 */

#include <system.h>
#include <paging.h>
#include <stdio.h>
#include <mm.h>

#define pagetable_addr(number, pagedir) \
    ((u32_t *)((pagedir)[(number)/0x400] & 0xfffff000))

#define DEBUG_MOUNT_MEMORY 0

u32_t *mpagedir;

extern size_t memory_size;	/* Размер памяти (в Kb) */
extern size_t memory_used;

inline void enable_paging(u32_t * pagedir)
{
  /*
   * Поместим адрес каталога страниц в CR3. 
   * Включим страничную адресацию.
   */

  asm("movl %%eax, %%cr3\n" \
      "movl %%cr0, %%eax\n" \
      "orl $0x80000000, %%eax\n" \
      "movl %%eax, %%cr0": :"a"(pagedir));
}

void setup_paging(void)
{
  u32_t pd, i, j, pagedirs;

  /* выделим память под каталог страниц */
  mpagedir = (u32_t *) kmalloc(PAGE_SIZE);

  pagedirs = memory_size / 0x1000;
  if (memory_size & 0xfff)
    pagedirs++;

  for (i = 0; i < pagedirs; i++) {
    /* создадим таблицу страниц */
    pd = (u32_t) kmalloc(PAGE_SIZE);
    mpagedir[i] = pd | 3;

    /* смонтируем на неё страницы */
    for (j = 0; j < 1024; j++)
      ((u32_t *) pd)[j] = ((i * 1024 + j) * 0x1000) | 3;
  }

  enable_paging(mpagedir);
}

u32_t k_umount_page(register u32_t log_page, register u32_t * pagedir)
{
  u32_t *pagetable;
  u32_t page;
  pagetable = pagetable_addr(log_page, pagedir);
  /* Узнаём физический адрес страницы */
  page = pagetable[log_page & 0x3ff] & 0xfffff000;
#if DEBUG_MOUNT_MEMORY
  printk(" um(0x%X-x-0x%X) ", page, log_page);
#endif
  pagetable[log_page & 0x3ff] = 0;
  return (page);
}

u32_t k_mount_page(register u32_t phys_page, register u32_t log_page, register u32_t * pagedir, register u8_t c3wp)
{
  u32_t *pagetable;
  u8_t flags = 3;
  if (c3wp)
    flags = 7;

#if DEBUG_MOUNT_MEMORY
  printk("m(0x%X->0x%X) ", phys_page, log_page);
  printk("f=0x%X", flags);
#endif
  /*
   * Узнаем физический адрес таблицы, в которой будет расположена страница
   */
  pagetable = pagetable_addr(log_page, pagedir);
  /* Если pagetable не существует - создаём её */
  if (!pagetable) {
#if DEBUG_MOUNT_MEMORY
    printk(" mk pgtable ");
#endif
    pagetable = (u32_t *) kmalloc(PAGE_SIZE);
    pagedir[log_page / 0x400] = (u32_t) pagetable | flags;
  }
#if DEBUG_MOUNT_MEMORY
  printk(" pt=0x%X\n", pagetable);
#endif
  pagetable[log_page & 0x3ff] = (phys_page * 0x1000) | flags;
  return ((unsigned long)pagetable);
}
