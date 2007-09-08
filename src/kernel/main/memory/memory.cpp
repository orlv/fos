/*
  kernel/main/memory/memory.cpp
  Copyright (C) 2007 Oleg Fedorov
*/

#include <fos/mm.h>
#include <fos/printk.h>
#include <fos/process.h>
#include <fos/hal.h>
#include <fos/pager.h>


void mm_srv()
{
  Thread *thread;
  struct message *msg = new message;
  msg->tid = 0;
  while (1) {
    msg->recv_size = 0;
    msg->tid = _MSG_SENDER_ANY;

    receive(msg);

    switch(msg->a0){
    case MM_CMD_MEM_ALLOC:
      //printk("mm: allocating 0x%X bytes of memory\n", msg->a1);
      thread = hal->procman->get_thread_by_tid(msg->tid);
      if(!msg->a2)
	msg->a0 = (u32_t) thread->process->memory->mem_alloc(msg->a1);
      else
	msg->a0 = get_lowpage() * PAGE_SIZE;
      msg->send_size = 0;
      reply(msg);
      break;

    case MM_CMD_MEM_MAP:
      //printk("mm: mapping 0x%X bytes of memory to 0x%X\n", msg->a2, msg->a1);
      thread = hal->procman->get_thread_by_tid(msg->tid);
      msg->a0 = (u32_t) thread->process->memory->mem_alloc_phys(msg->a1, msg->a2);
      msg->send_size = 0;
      reply(msg);
      break;

    case MM_CMD_MEM_FREE:
      //printk("mm: freeing 0x%X bytes from 0x%X\n", msg-a2, msg->a1);
      thread = hal->procman->get_thread_by_tid(msg->tid);
      if(msg->a1 > USER_MEM_BASE){
	msg->a0 = 1;
	thread->process->memory->mem_free((void *)msg->a1, msg->a2);
      } else
	msg->a0 = -1;
      msg->send_size = 0;
      reply(msg);
      break;
      
    default:
      msg->a0 = RES_FAULT;
      msg->send_size = 0;
      reply(msg);
    }
  }
}

Memory::Memory(offs_t base, size_t size)
{
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
  /*  list_for_each_safe (curr, n, UsedMem) {
    pager->umap_pages(PAGE(curr->item->vptr), curr->item->size / PAGE_SIZE);
    delete curr->item;
    delete curr;
  }

  delete UsedMem->item;
  delete UsedMem;*/

#warning добавить освобождение выделенных страниц
  
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

/* смонтировать набор физических страниц, выданных kmalloc() в любую область памяти */
void *Memory::kmem_alloc(register void *kmem_address, register size_t size)
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

  void *ptr = this->mem_alloc(phys_pages, pages_cnt);
  
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

  __mt_disable();
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
  }

  u32_t vptr = p->vptr;

  if (p->size > size) {	/* используем только часть блока */
    p->vptr += size;	/* скорректируем указатель на начало блока */
    p->size -= size;	/* вычтем выделяемый размер */
  } else {			/* (p->size == block->size) */
    /* при выделении используем весь блок (запись о нём удаляется из списка свободных блоков) */
    if(curr == FreeMem)
      FreeMem = curr->next;
    delete curr->item;
    delete curr;
  }

  pager->map_pages(phys_pages, PAGE(vptr), pages_cnt);
    
  __mt_enable();

  return (void *)vptr;
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
  u32_t vptr = (u32_t) log_address;

  __mt_disable();
  List<memblock *> *curr = FreeMem;
  
  while(1) {
    p = curr->item;
    if ((p->vptr <= vptr) && (p->vptr + p->size >= vptr + size)) {
      break;
    }
    curr = curr->next;
    if ((curr == FreeMem) || (p->vptr >= vptr)) {
      __mt_enable();
      printk("TaskMem: can't map %d bytes to 0x%X!\n", size, log_address);
      return 0;
    }
  }

  if (p->vptr == vptr) {
    if (p->size == size) {
      if(curr == FreeMem)
	FreeMem = curr->next;
      delete curr->item;
      delete curr;
    } else {
      p->vptr += size;
      p->size -= size;
    }
  } else {
    if (p->vptr + p->size > vptr + size) {
      memblock *b = new memblock;
      b->vptr = vptr + size;
      b->size = p->size - size - (vptr - p->vptr);
      curr->add(b);
    }
    p->size = vptr - p->vptr;
  }

  pager->map_pages(phys_pages, PAGE(vptr), pages_cnt);

  __mt_enable();

  return (void *) vptr;
}

void Memory::mem_free(register void *ptr, register size_t size)
{
  size = ((size + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
  u32_t vptr = (u32_t) ptr;

  __mt_disable();  
  /* проверим, смонтированы ли страницы */
  if(!check_pages(PAGE(vptr), pager->pagedir, PAGE(size))) {
    __mt_enable();
    printk("Trying to delete non-allocated page(s)!\n");
    return;
  }
   
  //printk("Memory: freeing 0x%X bytes, starting (virtual) 0x%X\n", size, ptr);
  pager->umap_pages(PAGE(vptr), PAGE(size));

  List<memblock *> *curr = FreeMem;
  memblock *c;
  memblock *next;

  /* ищем, куда добавить блок */
  if(FreeMem->item->vptr > vptr + size) {
    memblock *p = new memblock;
    p->vptr = vptr;
    p->size = size;
    FreeMem = FreeMem->add_tail(p);
    c = FreeMem->item;
    __mt_enable();
    return;
  }

  do{
    c = curr->item;
    /* слить с верхним соседом */
    if (c->vptr == vptr + size) {
      c->vptr = vptr;
      c->size += size;
      __mt_enable();
      return;
    }
    
    /* слить с нижним соседом */
    if (c->vptr + c->size == vptr) {
      c->size += size;

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
    if ((c->vptr + c->size < vptr) && ((curr->next == FreeMem) ||  (next->vptr > vptr + size))) {
      memblock *p = new memblock;
      p->vptr = vptr;
      p->size = size;
      curr->add(p);
      __mt_enable();
      return;
    }
    
    curr = curr->next;
  }while (curr != FreeMem);

  memblock *p = new memblock;
  p->vptr = vptr;
  p->size = size;
  curr->add_tail(p);
  __mt_enable();
}

