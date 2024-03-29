/*
  kernel/main/memory/pager.cpp
  Copyright (C) 2007 Oleg Fedorov
*/

#include <fos/pager.h>
#include <fos/printk.h>

Pager::Pager(u32_t pagedir, u16_t flags)
{
  this->pagedir = (u32_t *) pagedir;
  this->flags = flags;
}

Pager::~Pager()
{
  /* освободим таблицы страниц и каталог страниц */
  for (u32_t i = USER_MEM_BASE/1024; i < 1024; i++) {
    if(!pagedir[i])
      continue;

    u32_t *pagetable = (u32_t *) (kmem_log_addr(PAGE(pagedir[i])) * PAGE_SIZE);

    for(u32_t j=0; j<1024; j++) {
      if(pagetable[j])
	put_page(PAGE(pagetable[j]));
    }

    put_page(PAGE(OFFSET(pagetable)));
    kfree(pagetable, PAGE_SIZE);
  }

  put_page(PAGE(OFFSET(pagedir)));
  kfree(pagedir, PAGE_SIZE);
}

u32_t Pager::mount_page(register u32_t phys_page, register u32_t log_page)
{
  return map_page(phys_page, log_page, pagedir, flags);
}

u32_t Pager::umount_page(register u32_t log_page)
{
  return umap_page(log_page, pagedir);
}

void Pager::map_pages(register u32_t *phys_pages, register u32_t log_page, register size_t n)
{
  for (size_t i = 0; i < n; i++) {
    mount_page(phys_pages[i], log_page);
    log_page++;
  }
}

void Pager::umap_pages(register u32_t log_page, register size_t n)
{
  for (size_t i = 0; i < n ; i++) {
    umount_page(log_page);
    log_page++;
  }
}
