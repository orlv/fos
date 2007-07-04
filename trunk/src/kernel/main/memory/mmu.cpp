/*
 * kernel/memory/mmu.cpp
 * Copyright (C) 2005-2007 Oleg Fedorov
 */

#include <system.h>
#include <mmu.h>
#include <stdio.h>
#include <mm.h>
#include <hal.h>

void enable_paging(u32_t * pagedir)
{
  /*
   * Поместим адрес каталога страниц в CR3. 
   * Включим страничную адресацию.
   */

  __asm__ __volatile__("movl %%eax, %%cr3\n"	\
		       "movl %%cr0, %%eax\n"	\
		       "orl $0x80000000, %%eax\n"		\
		       "movl %%eax, %%cr0": :"a"(pagedir));
}

u32_t umap_page(register u32_t log_page, register u32_t * pagedir)
{
  u32_t page;
  //printk("um lp=[0x%X], pd=[0x%X]\n", log_page, pagedir);
  u32_t *pagetable = (u32_t *)(kmem_log_addr(PAGE((u32_t) pagetable_addr(log_page, pagedir))) * PAGE_SIZE);

  /* Узнаём физический адрес страницы */
  if((page = (pagetable[log_page & 0x3ff] & 0xfffff000))){
     put_page(PAGE(page));
  }

  //printk("* log_pt=[0x%X], pys_page=0x%X \n ", pagetable, page);
  pagetable[log_page & 0x3ff] = 0;
  flush_tlb_single(log_page*PAGE_SIZE);
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
  __mt_disable();
  u32_t *pagetable = pagetable_addr(log_page, pagedir);
  /* Если pagetable не существует - создаём её */
  //printk("p=[0x%X], l=[0x%X], pd=[0x%X], pt=[0x%X]\n", phys_page, log_page, pagedir, pagetable);
  if (!pagetable) {
    /* в pagedir ядра все таблицы уже смонтированы, так что можно не беспокоиться
       по поводу обращения в эту часть кода внутри kmalloc() */
    //while(1) asm("incb 0xb8000+156\n" "movb $0x2f,0xb8000+157 ");
    pagetable = (u32_t *) kmalloc(PAGE_SIZE); /* таблица страниц должна быть доступна из любого адресного
						 пространства - поэтому используем kmalloc() (не забываем,
						 что kmalloc() возвращает логический адрес) */
    //printk("+pt=[0x%X] ", pagetable);
    pagedir[log_page / 0x400] = (u32_t)kmem_phys_addr(PAGE((u32_t) pagetable)) | flags;
    //printk("pd(0x%X) ", pagedir[log_page / 0x400]);
  } else {
    /* выясним, по какому адресу таблица смонтирована в памяти ядра */
    pagetable = (u32_t *)(kmem_log_addr(PAGE((u32_t) pagetable)) * PAGE_SIZE);
    //printk("pt=[0x%X]\n", pagetable);
  }
  __mt_enable();  
  pagetable[log_page & 0x3ff] = (phys_page * PAGE_SIZE) | flags;
  flush_tlb_single(log_page*PAGE_SIZE);
  //printk("{0x%X} \n", pagetable[log_page & 0x3ff]);
  return ((unsigned long)pagetable);
}
