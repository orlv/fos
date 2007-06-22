/*
  kernel/main/memory/memory.cpp
  Copyright (C) 2007 Oleg Fedorov
*/

#include <stdio.h>
#include <mm.h>
#include <process.h>
#include <hal.h>
#include <paging.h>

Memory::Memory(offs_t base, size_t size, u16_t flags)
{
  this->flags = flags;

  memblock *block = new memblock;

  block->vptr = base;
  block->size = size;

  FreeMem = new List<memblock *>(block);
}

#if 0
u32_t *Memory::CreatePageDir()
{
  u32_t i;
  u32_t *pagedir;

  /* выделим память под каталог страниц */
  pagedir = (u32_t *) get_page() * PAGE_SIZE;

  for (i = 0; i < USER_MEM_BASE/1024; i++) {
    pagedir[i] = hal->ProcMan->kPageDir[i];
  }

  return pagedir;
}
#endif

u32_t Memory::mount_page(register u32_t phys_page, register u32_t log_page)
{
  return map_page(phys_page, log_page, pagedir, flags);
}

u32_t Memory::umount_page(register u32_t log_page)
{
  return umap_page(log_page, pagedir);
}

void *Memory::mem_alloc(register size_t size)
{
  size_t pages_cnt = size - (size % MM_MINALLOC);
  if (size % MM_MINALLOC)
    pages_cnt += MM_MINALLOC;
  pages_cnt /= PAGE_SIZE;

  u32_t *phys_pages = new u32_t[pages_cnt];
  size_t i;
  for(i = 0; i < pages_cnt; i++){
    phys_pages[i] = get_page();
  }

  void *ptr =  this->mem_alloc(phys_pages, pages_cnt);

  if(!ptr){
    for(i = 0; i < pages_cnt; i++){
      put_page(PAGE(phys_pages[i]));
    }
  }

  delete phys_pages;
  
  return ptr;
}

void *Memory::mem_alloc_phys(register u32_t phys_address, register size_t size)
{
  size_t pages_cnt = size - (size % MM_MINALLOC);
  if (size % MM_MINALLOC)
    pages_cnt += MM_MINALLOC;
  pages_cnt /= PAGE_SIZE;

  u32_t *phys_pages = new u32_t[pages_cnt];
  size_t i;
  for(i = 0; i < pages_cnt; i++){
    phys_pages[i] = phys_address / PAGE_SIZE;
    phys_address += PAGE_SIZE;
  }

  void *res = mem_alloc(phys_pages, pages_cnt);
  delete phys_pages;
  return res;
}

/* смонтировать набор физических страниц в любое свободное место */
void *Memory::mem_alloc(register u32_t *phys_pages, register size_t pages_cnt)
{
  memblock *p;
  size_t size = pages_cnt * PAGE_SIZE;
  List<memblock *> *curr  = FreeMem;
  
  /* ищем свободный блок подходящего размера */
  while(1){
    p = curr->item;
    if (p->size >= size) {
      break;
    }
    curr = curr->next;
    if(curr == FreeMem){
      printk("TaskMem: can't allocate %d bytes of memory!\n", size);
      return 0;
    }
  };

  offs_t block;
  
  if (p->size > size) {	/* используем только часть блока */
    block = p->vptr;
    p->vptr += size;		/* скорректируем указатель на начало блока */
    p->size -= size;		/* вычтем выделяемый размер */
  } else {			/* (p->size == size) */
    /* при выделении используем весь блок (запись о нём удаляется из списка свободных блоков) */
    block = p->vptr;
    delete curr->item;
    delete curr;
  }

  map_pages(phys_pages, PAGE(block), pages_cnt);

  memblock *ublock = new memblock;
  ublock->phys_pages = phys_pages;
  ublock->vptr = block;
  ublock->size = size;
    
  if (UsedMem)
    UsedMem->add_tail(ublock);
  else
    UsedMem = new List<memblock *>(ublock);
  
  return (void *)block;
}

/* выделить набор физических страниц и смонтировать в конкретное место */
void *Memory::mmap(register size_t size, register void *log_address)
{
  size_t pages_cnt = size - (size % MM_MINALLOC);
  if (size % MM_MINALLOC)
    pages_cnt += MM_MINALLOC;
  pages_cnt /= PAGE_SIZE;

  u32_t *phys_pages = new u32_t[pages_cnt];
  size_t i;

  for(i = 0; i < pages_cnt; i++){
    phys_pages[i] = get_page() * PAGE_SIZE;
  }

  void *ptr = do_mmap(phys_pages, log_address, pages_cnt);

  if(!ptr){
    for(i = 0; i < pages_cnt; i++){
      kfree((void *)(phys_pages[i] * PAGE_SIZE));
    }
  }

  delete phys_pages;
  
  return ptr;
}

/* смонтировать набор физических страниц, выданных kmalloc() в конкретную область памяти */
void *Memory::kmmap(register void *kmem_address, register void *log_address, register size_t size)
{
  size_t pages_cnt = size - (size % MM_MINALLOC);
  if (size % MM_MINALLOC)
    pages_cnt += MM_MINALLOC;
  pages_cnt /= PAGE_SIZE;

  u32_t *phys_pages = new u32_t[pages_cnt];
  size_t i;

  for(i = 0; i < pages_cnt; i++){
    phys_pages[i] = PAGE((u32_t)kmem_phys_addr(PAGE((u32_t)kmem_address)));
    kmem_address = (void *) ((u32_t)kmem_address + PAGE_SIZE);
  }

  void *res = do_mmap(phys_pages, log_address, pages_cnt);
  delete phys_pages;
  return res;
}



/* смонтировать набор физических страниц в конкретную область памяти */
void *Memory::mmap(register void *phys_address, register void *log_address, register size_t size)
{
  size_t pages_cnt = size - (size % MM_MINALLOC);
  if (size % MM_MINALLOC)
    pages_cnt += MM_MINALLOC;
  pages_cnt /= PAGE_SIZE;

  u32_t *phys_pages = new u32_t[pages_cnt];
  size_t i;

  for(i = 0; i < pages_cnt; i++){
    phys_pages[i] = PAGE((u32_t)phys_address);
    phys_address = (void *) ((u32_t)phys_address + PAGE_SIZE);
  }

  void *res = do_mmap(phys_pages, log_address, pages_cnt);
  delete phys_pages;
  return res;
}

/* смонтировать набор физических страниц в конкретную область памяти */
void *Memory::do_mmap(register u32_t *phys_pages, register void *log_address, register size_t pages_cnt)
{
  memblock *p;
  size_t size = pages_cnt * PAGE_SIZE;
  List<memblock *> *curr = FreeMem;
  
  while(1) {
    p = curr->item;
    if ((p->vptr <= (u32_t) log_address) && (p->vptr + p->size >= (u32_t) log_address + size)) {
      break;
    }
    curr = curr->next;
    if ((curr == FreeMem) || (p->vptr >= (u32_t) log_address)) {
      printk("TaskMem: can't map %d bytes to 0x%X!\n", size, log_address);
      return 0;
    }
  };

  if (p->vptr == (u32_t) log_address) {
    if (p->size == size) {
      delete curr->item;
      delete curr;
    } else {
      p->vptr += size;
      p->size -= size;
    }
  } else {
    size_t asize = p->size;
    p->size = (u32_t) log_address - (u32_t) p->vptr;
    if (asize + p->vptr > (u32_t) log_address + size) {
      memblock *b = new memblock;
      b->vptr = (u32_t) log_address + size;
      b->size = asize - size - p->size;
      curr->add(b);
    }
  }

  map_pages(phys_pages, PAGE((u32_t)log_address), pages_cnt);
  
  memblock *ublock = new memblock;
  ublock->phys_pages = phys_pages;
  ublock->vptr = (u32_t) log_address;
  ublock->size = size;

  if (UsedMem)
    UsedMem->add_tail(ublock);
  else
    UsedMem = new List<memblock *>(ublock);

  return log_address;
}

void Memory::map_pages(register u32_t *phys_pages, register u32_t log_page, register size_t n)
{
  for (u32_t i = 0; i < n; i++) {
    //printk("[0x%X]->[0x%X]\n", phys_pages[i], log_page);
    mount_page(phys_pages[i], log_page);
    log_page++;
  }
}

void Memory::umap_pages(register u32_t *log_pages, register size_t n)
{
  for (u32_t i = 0; i < n ; i++) {
    umount_page(log_pages[i]);
  }
}


void Memory::mem_free(register void *ptr)
{
  List<memblock *> *curr = UsedMem;
  memblock *p;

  /* выяснить size */
  while(1) {
    p = curr->item;
    if (p->vptr == (u32_t) ptr) {
      delete curr;
      break;
    }

    curr = curr->next;
    if (curr == UsedMem) {
      printk("Deleting non-allocated page!\n");
      return;
    }
  }

  umap_pages(p->phys_pages, p->size / PAGE_SIZE);
  /*for(u32_t i = 0; i < p->size / PAGE_SIZE; i++){
    kfree((void *)(p->phys_pages[i] * PAGE_SIZE));
    }*/

  curr = FreeMem;

  memblock *c;
  memblock *next;
  /* ищем, куда добавить блок */
  do {
    c = curr->item;

    /* слить с верхним соседом */
    if (c->vptr == p->vptr + p->size) {
      c->vptr = p->vptr;
      c->size += p->size;
      delete p;
      return;
    }

    /* слить с нижним соседом */
    if ((c->vptr + c->size == p->vptr)) {
      c->size += p->size;
      delete p;

      next = curr->next->item;
      /* и нижнего с верхним соседом */
      if (c->vptr + c->size == next->vptr) {
	c->size += next->size;
	delete next;
	delete curr->next;
      }
      return;
    }

    next = curr->next->item;
    /* разместить между нижним и верхним соседями */
    if ((c->vptr + c->size < p->vptr) && (next->vptr > p->vptr + p->size)) {
      curr->add(p);
      return;
    }

    curr = curr->next;
  } while (curr != FreeMem);

  FreeMem->add_tail(p);
}
