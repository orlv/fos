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

void *Memory::mem_alloc(register size_t size)
{
  size_t pages_cnt = size - (size % MM_MINALLOC);
  if (size % MM_MINALLOC)
    pages_cnt += MM_MINALLOC;
  pages_cnt /= PAGE_SIZE;

  u32_t *phys_pages = new u32_t[pages_cnt];
  for(size_t i = 0; i < pages_cnt; i++){
    phys_pages[i] = get_page();
  }

  void *ptr =  this->mem_alloc(phys_pages, pages_cnt);

  if(!ptr){
    for(size_t i = 0; i < pages_cnt; i++){
      put_page(phys_pages[i]);
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
  u32_t phys_page = PAGE(phys_address);

  for(size_t i = 0; i < pages_cnt; i++){
    phys_pages[i] = phys_page;
    phys_page++;
  }

  void *ptr = mem_alloc(phys_pages, pages_cnt);

  if(!ptr){
    for(size_t i = 0; i < pages_cnt; i++){
      put_page(phys_pages[i]);
    }
  }

  delete phys_pages;
  return ptr;
}

/* смонтировать набор физических страниц в любое свободное место */
void *Memory::mem_alloc(register u32_t *phys_pages, register size_t pages_cnt)
{
  memblock *p;
  size_t size = pages_cnt * PAGE_SIZE;
  volatile List<memblock *> *curr  = FreeMem;

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
  }

  memblock *block = new memblock;
  block->vptr = p->vptr;
  block->size = size;
  block->phys_pages = phys_pages;

  if (p->size > block->size) {	/* используем только часть блока */
    p->vptr += block->size;	/* скорректируем указатель на начало блока */
    p->size -= block->size;	/* вычтем выделяемый размер */
  } else {			/* (p->size == block->size) */
    /* при выделении используем весь блок (запись о нём удаляется из списка свободных блоков) */
    if(curr == FreeMem)
      FreeMem = curr->next;
    delete curr->item;
    delete curr;
  }
  
  map_pages(phys_pages, PAGE(block->vptr), pages_cnt);
    
  if (UsedMem)
    UsedMem->add_tail(block);
  else
    UsedMem = new List<memblock *>(block);

  return (void *)block->vptr;
}

/* выделить набор физических страниц и смонтировать в конкретное место */
void *Memory::mmap(register size_t size, register void *log_address)
{
  size_t pages_cnt = size - (size % MM_MINALLOC);
  if (size % MM_MINALLOC)
    pages_cnt += MM_MINALLOC;
  pages_cnt /= PAGE_SIZE;

  u32_t *phys_pages = new u32_t[pages_cnt];

  for(size_t i = 0; i < pages_cnt; i++){
    phys_pages[i] = get_page() * PAGE_SIZE;
  }

  void *ptr = do_mmap(phys_pages, log_address, pages_cnt);

  if(!ptr){
    for(size_t i = 0; i < pages_cnt; i++){
      put_page(phys_pages[i]);
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
  u32_t kmem_page = PAGE((u32_t) kmem_address);
  for(size_t i = 0; i < pages_cnt; i++){
    phys_pages[i] = PAGE((u32_t)kmem_phys_addr(kmem_page));
    kmem_page++;
  }

  void *ptr = do_mmap(phys_pages, log_address, pages_cnt);

  if(!ptr){
    for(size_t i = 0; i < pages_cnt; i++){
      put_page(phys_pages[i]);
    }
  }

  delete phys_pages;
  return ptr;
}

/* смонтировать набор физических страниц в конкретную область памяти */
void *Memory::mmap(register void *phys_address, register void *log_address, register size_t size)
{
  size_t pages_cnt = size - (size % MM_MINALLOC);
  if (size % MM_MINALLOC)
    pages_cnt += MM_MINALLOC;
  pages_cnt /= PAGE_SIZE;

  u32_t *phys_pages = new u32_t[pages_cnt];
  u32_t phys_page = PAGE((u32_t) phys_address);

  for(size_t i = 0; i < pages_cnt; i++){
    phys_pages[i] = phys_page;
    phys_page++;
  }

  void *ptr = do_mmap(phys_pages, log_address, pages_cnt);

  if(!ptr && phys_address){
    for(size_t i = 0; i < pages_cnt; i++){
      put_page(phys_pages[i]);
    }
  }
  
  delete phys_pages;
  return ptr;
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
  }

  memblock *block = new memblock;
  block->phys_pages = phys_pages;
  block->vptr = (u32_t) log_address;
  block->size = size;
  
  if (p->vptr == block->vptr) {
    if (p->size == block->size) {
      if(curr == FreeMem)
	FreeMem = curr->next;
      delete curr->item;
      delete curr;
    } else {
      p->vptr += block->size;
      p->size -= block->size;
    }
  } else {
    if (p->vptr + p->size > block->vptr + block->size) {
      memblock *b = new memblock;
      b->vptr = block->vptr + block->size;
      b->size = p->size - block->size - (block->vptr - p->vptr);
      curr->add(b);
    }
    p->size = block->vptr - p->vptr;
  }

  map_pages(phys_pages, PAGE((u32_t)log_address), pages_cnt);
  

  if (UsedMem)
    UsedMem->add_tail(block);
  else
    UsedMem = new List<memblock *>(block);

  return (void *) block->vptr;
}

u32_t Memory::mount_page(register u32_t phys_page, register u32_t log_page)
{
  return map_page(phys_page, log_page, pagedir, flags);
}

u32_t Memory::umount_page(register u32_t log_page)
{
  return umap_page(log_page, pagedir);
}

void Memory::map_pages(register u32_t *phys_pages, register u32_t log_page, register size_t n)
{
  for (size_t i = 0; i < n; i++) {
    mount_page(phys_pages[i], log_page);
    log_page++;
  }
}

void Memory::umap_pages(register u32_t log_page, register size_t n)
{
  for (size_t i = 0; i < n ; i++) {
    umount_page(log_page);
    log_page++;
  }
}

void Memory::dump_used()
{
  List<memblock *> *curr = UsedMem;
  memblock *p;
  
  while(1) {
    p = curr->item;
    printk("{vptr=0x%X, size=0x%X. um=0x%X} \n", p->vptr, p->size, UsedMem);
    curr = curr->next;
    if (curr == UsedMem) {
      return;
    }
  }
  
}

void Memory::dump_free()
{
  List<memblock *> *curr = FreeMem;
  memblock *p;
  u32_t i=0;  
  while(i<5) {
    i++;
    p = curr->item;
    printk("{vptr=0x%X, size=0x%X. fm=0x%X} \n", p->vptr, p->size, FreeMem);
    curr = curr->next;
    if (curr == FreeMem) {
      return;
    }
  }
  printk("overflow \n");
  
}

void Memory::mem_free(register void *ptr)
{
  List<memblock *> *curr = UsedMem;
  memblock *p;

  if(!UsedMem){
    printk("No one page was allocated, nothing to free!\n");
    return;
  }
  
  /* выяснить size */
  while(1) {
    p = curr->item;
    if (p->vptr == (u32_t) ptr) {
      if(curr == UsedMem){
	if(curr != curr->next)
	  UsedMem = curr->next;
	else
	  UsedMem = 0;
      }
      delete curr;
      break;
    }

    curr = curr->next;
    if (curr == UsedMem) {
      printk("Deleting non-allocated page!\n");
      return;
    }
  }

  umap_pages(PAGE(p->vptr), p->size / PAGE_SIZE);

  curr = FreeMem;
  memblock *c;
  memblock *next;

  /* ищем, куда добавить блок */
  if(FreeMem->item->vptr > p->vptr + p->size) {
    FreeMem = FreeMem->add_tail(p);
    c = FreeMem->item;
    return;
  }

  do{
    c = curr->item;
    /* слить с верхним соседом */
    if (c->vptr == p->vptr + p->size) {
      c->vptr = p->vptr;
      c->size += p->size;
      delete p;
      return;
    }
    
    /* слить с нижним соседом */
    if (c->vptr + c->size == p->vptr) {
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
    }while (curr != FreeMem);

    next = curr->next->item;
    /* разместить между нижним и верхним соседями */
    if ((c->vptr + c->size < p->vptr) && ((curr->next == FreeMem) ||  (next->vptr > p->vptr + p->size))) {
      curr->add(p);
      return;
    }
    
    curr = curr->next;
  }while (curr != FreeMem);

  curr->add_tail(p);
}
