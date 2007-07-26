/*
  kernel/main/memory/memory.cpp
  Copyright (C) 2007 Oleg Fedorov
*/

#include <fos/mm.h>
#include <fos/printk.h>
#include <fos/process.h>
#include <fos/hal.h>
#include <fos/pager.h>

Memory::Memory(offs_t base, size_t size, u16_t flags)
{
  this->flags = flags;

  memblock *block = new memblock;

  block->vptr = base;
  block->size = size;

  FreeMem = new List<memblock *>(block);

  mem_base = base;
}

Memory::~Memory()
{
  List<memblock *> *curr, *n;

  /* удалим список использованной памяти и освободим выделенные страницы */
  list_for_each_safe (curr, n, UsedMem) {
    pager->umap_pages(PAGE(curr->item->vptr), curr->item->size / PAGE_SIZE);
    delete curr->item;
    delete curr;
  }

  delete UsedMem->item;
  delete UsedMem;

  /* удалим список свободной памяти */
  list_for_each_safe (curr, n, FreeMem) {
    delete curr->item;
    delete curr;
  }

  delete FreeMem->item;
  delete FreeMem;

  delete pager;
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
  __mt_disable();
  block->vptr = p->vptr;
  block->size = size;
  //block->phys_pages = phys_pages;

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
  
  pager->map_pages(phys_pages, PAGE(block->vptr), pages_cnt);
    
  if (UsedMem)
    UsedMem->add_tail(block);
  else
    UsedMem = new List<memblock *>(block);

  __mt_enable();

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

  __mt_disable();
  List<memblock *> *curr = FreeMem;
  
  while(1) {
    p = curr->item;
    if ((p->vptr <= (u32_t) log_address) && (p->vptr + p->size >= (u32_t) log_address + size)) {
      break;
    }
    curr = curr->next;
    if ((curr == FreeMem) || (p->vptr >= (u32_t) log_address)) {
      __mt_enable();
      printk("TaskMem: can't map %d bytes to 0x%X!\n", size, log_address);
      return 0;
    }
  }

  memblock *block = new memblock;
  //block->phys_pages = phys_pages;
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
  
  pager->map_pages(phys_pages, PAGE((u32_t)log_address), pages_cnt);
  

  if (UsedMem)
    UsedMem->add_tail(block);
  else
    UsedMem = new List<memblock *>(block);

  __mt_enable();

  return (void *) block->vptr;
}

void Memory::mem_free(register void *ptr)
{
  List<memblock *> *curr = UsedMem;
  memblock *p;

  if(!UsedMem){
    printk("No one page was allocated, nothing to free!\n");
    return;
  }
  __mt_disable();  
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
      __mt_enable();
      printk("Deleting non-allocated page!\n");
      return;
    }
  }
  //printk("Memory: freeing 0x%X bytes, starting (virtual) 0x%X\n", p->size, p->vptr);
  pager->umap_pages(PAGE(p->vptr), p->size / PAGE_SIZE);

  curr = FreeMem;
  memblock *c;
  memblock *next;

  /* ищем, куда добавить блок */
  if(FreeMem->item->vptr > p->vptr + p->size) {
    FreeMem = FreeMem->add_tail(p);
    c = FreeMem->item;
    __mt_enable();
    return;
  }

  do{
    c = curr->item;
    /* слить с верхним соседом */
    if (c->vptr == p->vptr + p->size) {
      c->vptr = p->vptr;
      c->size += p->size;
      delete p;
      __mt_enable();
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
      __mt_enable();
      return;
    }while (curr != FreeMem);

    next = curr->next->item;
    /* разместить между нижним и верхним соседями */
    if ((c->vptr + c->size < p->vptr) && ((curr->next == FreeMem) ||  (next->vptr > p->vptr + p->size))) {
      curr->add(p);
      __mt_enable();
      return;
    }
    
    curr = curr->next;
  }while (curr != FreeMem);

  curr->add_tail(p);
  __mt_enable();
}
