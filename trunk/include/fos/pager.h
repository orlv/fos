/*
    fos/pager.h
    Copyright (C) 2005-2007 Oleg Fedorov
*/

#ifndef _FOS_PAGER_H
#define _FOS_PAGER_H

#include <types.h>
#include <fos/mmu.h>
#include <fos/system.h>
#include <fos/page.h>

class Pager {
 public:
  Pager(u32_t pagedir, u16_t flags);
  ~Pager();
  u32_t *pagedir; /* каталог страниц */
  u16_t flags;

  u32_t mount_page(register u32_t phys_page, register u32_t log_page);
  u32_t umount_page(register u32_t log_page);

  void map_pages(register u32_t *phys_pages, register u32_t log_page, register size_t n);
  void umap_pages(register u32_t log_page, register size_t n);
};

/* возвращает физический адрес страницы */
static inline u32_t page_phys_address(register u32_t log_page, register u32_t * pagedir, u32_t *pagetable)
{
  return (pagetable[log_page & 0x3ff] & 0xfffff000);
}

/* проверяет, все ли страницы из последовательности находятся в указанном адресном пространстве */
static inline u32_t check_pages(register u32_t log_page, register u32_t * pagedir, register size_t count)
{
  u32_t *pagetable = 0;

  __mt_disable();

  while(count) {
    if(!pagetable) {
      pagetable = (u32_t *)(kmem_log_addr(PAGE(OFFSET(pagetable_addr(log_page, pagedir)))) * PAGE_SIZE);
      if(!pagetable) {
	__mt_enable();
	return 0;
      }
    }
    
    if(!page_phys_address(log_page, pagedir, pagetable)){
      __mt_enable();
      return 0;
    }
    count--;
    log_page++;

    if(!(log_page & 0x3ff)) /* если страница находится в следующей таблице страниц - сбрасываем pagetable */
      pagetable = 0;
  }
  
  __mt_enable();
  return 1;
}


void put_page(u32_t page);
u32_t get_page();

/* DMA16_Pages - страницы, расположенные ниже 16 Мб */
void put_page_DMA16(u32_t page);
u32_t get_page_DMA16();

void enable_paging(u32_t * pagedir);

#endif
