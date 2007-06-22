/*
 * kernel/memory/mmu.cpp
 * Copyright (C) 2005-2007 Oleg Fedorov
 */

#include <system.h>
#include <paging.h>
#include <stdio.h>
#include <mm.h>
#include <hal.h>

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
  u32_t page;
  //printk("[0x%X]\n", log_page);
  //printk("[0x%X]\n", pagetable_addr(log_page, pagedir));
  //while(1);
  u32_t *pagetable = (u32_t *)(kmem_log_addr(PAGE((u32_t) pagetable_addr(log_page, pagedir))) * PAGE_SIZE);

  /* Узнаём физический адрес страницы */
  if((page = pagetable[log_page & 0x3ff] & 0xfffff000)){
    put_page(PAGE(page));
  }
  pagetable[log_page & 0x3ff] = 0;
  return (page);
}

u32_t map_page(register u32_t phys_page, register u32_t log_page, register u32_t * pagedir, register u16_t flags)
{
  alloc_page(phys_page);
  /* если страница монтируется в область ядра - указываем в свойствах страницы её логический адрес */
  if(log_page < KERNEL_MEM_LIMIT){
    kmem_set_log_addr(phys_page, log_page);
  }
  /*
   * Узнаем физический адрес таблицы, в которой будет расположена страница
   */
  u32_t *pagetable = pagetable_addr(log_page, pagedir);
  /* Если pagetable не существует - создаём её */
  printk("pd=[0x%X], p=[0x%X], l=[0x%X], pt=[0x%X]\n", pagedir, phys_page, log_page, pagetable);
  if (!pagetable) {
    /* в pagedir ядра все таблицы уже смонтированы, так что можно не беспокоиться
       по поводу обращения в эту часть кода внутри kmalloc() */
    //while(1) asm("incb 0xb8000+156\n" "movb $0x2f,0xb8000+157 ");
    pagetable = (u32_t *) kmalloc(PAGE_SIZE); /* таблица страниц должна быть доступна из любого адресного
						 пространства - поэтому используем kmalloc() (не забываем,
						 что kmalloc() возвращает логический адрес) */
    pagedir[log_page / 0x400] = (u32_t)kmem_phys_addr(PAGE((u32_t) pagetable)) | flags;
  } else {
    /* выясним, по какому адресу таблица смонтирована в памяти ядра */
    pagetable = (u32_t *)(kmem_log_addr(PAGE((u32_t) pagetable)) * PAGE_SIZE);
  }
  
  pagetable[log_page & 0x3ff] = (phys_page * PAGE_SIZE) | flags;
  return ((unsigned long)pagetable);
}
