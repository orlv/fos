/*
 * kernel/memory/mmu.cpp
 * Copyright (C) 2005-2007 Oleg Fedorov
 */

#include <fos/mmu.h>
#include <fos/pager.h>
#include <fos/fos.h>
#include <fos/printk.h>
#include <fos/mm.h>

void enable_paging(u32_t * pagedir)
{
  /*
   * Поместим адрес каталога страниц в CR3. 
   * Включим страничную адресацию.
   */

  __asm__ __volatile__("movl %%eax, %%cr3\n"			\
		       "movl %%cr0, %%eax\n"			\
		       "orl $0x80000000, %%eax\n"		\
		       "movl %%eax, %%cr0": :"a"(pagedir));
}

u32_t umap_page(register u32_t log_page, register u32_t * pagedir)
{
  u32_t page;
  //  printk("um lp=[0x%X], pd=[0x%X]\n", log_page, pagedir);
  preempt_disable();
  u32_t *pagetable = (u32_t *)(kmem_log_addr(PAGE(OFFSET(pagetable_addr(log_page, pagedir)))) * PAGE_SIZE);

  /* Узнаём физический адрес страницы */
  if((page = (pagetable[log_page & 0x3ff] & 0xfffff000)))
     put_page(PAGE(page));

  //  printk("* pt_log=[0x%X], page_ph=0x%X \n ", pagetable, page);
  pagetable[log_page & 0x3ff] = 0;
  flush_tlb_single(log_page*PAGE_SIZE);
  preempt_enable();
  return (page);
}

u32_t map_page(register u32_t phys_page, register u32_t log_page, register u32_t * pagedir, register u16_t flags)
{
  alloc_page(phys_page);
  /* если страница монтируется в область ядра - указываем в свойствах страницы её логический адрес */
  if(log_page < KERNEL_MEM_LIMIT)
    kmem_set_log_addr(phys_page, log_page);

  /*
   * Узнаем физический адрес таблицы, в которой будет расположена страница
   */
  preempt_disable();
  u32_t *pagetable = pagetable_addr(log_page, pagedir);
  /* Если pagetable не существует - создаём её */
  //printk("p=[0x%X], l=[0x%X], pd=[0x%X], pt=[0x%X]\n", phys_page, log_page, pagedir, pagetable);
  if (!pagetable) {
    /* в pagedir ядра все таблицы уже смонтированы, так что можно не беспокоиться
       по поводу обращения в эту часть кода внутри kmalloc() */

    pagetable = (u32_t *) kmalloc(PAGE_SIZE); /* таблица страниц должна быть доступна из любого адресного
						 пространства - поэтому используем kmalloc() (не забываем,
						 что kmalloc() возвращает логический адрес) */
    //printk("+pt_log=[0x%X] ", pagetable);
    pagedir[log_page / 0x400] = (u32_t)kmem_phys_addr(PAGE((u32_t) pagetable)) | flags;
    //printk("pt_ph(0x%X) ", pagedir[log_page / 0x400]);
  } else {
    /* выясним, по какому адресу таблица смонтирована в памяти ядра */
    pagetable = (u32_t *)(kmem_log_addr(PAGE(OFFSET(pagetable))) * PAGE_SIZE);
    //printk("pt_log=[0x%X]\n", pagetable);
  }
  pagetable[log_page & 0x3ff] = (phys_page * PAGE_SIZE) | flags;
  flush_tlb_single(log_page*PAGE_SIZE);
  preempt_enable();  
  //printk("{0x%X} \n", pagetable[log_page & 0x3ff]);
  return ((unsigned long)pagetable);
}

/* если страница выдана kmalloc() - возвратит её физический адрес */
u32_t * kmem_phys_addr(u32_t n)
{
  u32_t *pagetable = pagetable_addr(n, system->kmem->pager->pagedir);
  return (u32_t *) (pagetable[n & 0x3ff] & 0xfffff000);
}
