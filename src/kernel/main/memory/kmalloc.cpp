/*
 * kernel/memory/kmalloc.cpp
 * Copyright (C) 2007 Oleg Fedorov
 */

#include <mm.h>
#include <multiboot.h>
#include <paging.h>
#include <stdio.h>
#include <hal.h>

//size_t memory_used;

void put_page(u32_t page)
{
  if(!free_page(page)){ /* если эта страница больше никем не используется */
    memstack *m = new memstack;
    m->next = hal->free_page;
    m->n = page;
    hal->free_page = m;
    atomic_inc(&hal->free_pages);
  }
}

u32_t get_page()
{
  if(atomic_read(&hal->free_pages)){
    memstack *m;
    m = hal->free_page;
    hal->free_page =  hal->free_page->next;
    //alloc_page(m->n);
    atomic_dec(&hal->free_pages);
    return m->n;
  } else {
    return 0;
  }
}

void put_lowpage(u32_t page)
{
  if(!free_page(page)){ /* если эта страница больше никем не используется */
    memstack *m = new memstack;
    m->next = hal->free_lowpage;
    m->n = page;
    hal->free_lowpage = m;
    atomic_inc(&hal->free_lowpages);
  }
}

u32_t get_lowpage()
{
  memstack *m;
  m = hal->free_page;
  hal->free_page =  hal->free_page->next;
  //alloc_page(m->n);
  atomic_dec(&hal->free_pages);
  return m->n;
}


#define KERNEL_HEAP_SIZE 0x400000 /* 4Mb */
#define LOWMEM_SIZE 0x1000000 /* 16Mb*/
#define MIN_FREE_MEMORY 0x400000

void free(register void *ptr);

u32_t *kpagedir;

void init_memory()
{
  extern multiboot_info_t *__mbi;

  size_t memory_size = (__mbi->mem_upper + 1024)*1024;

  offs_t low_freemem_start = ((module_t *) __mbi->mods_addr)[__mbi->mods_count - 1].mod_end;
  if (low_freemem_start & 0xfff) {
    low_freemem_start &= 0xfffff000;
    low_freemem_start += PAGE_SIZE;
  }

  u32_t pages_cnt = PAGE(memory_size);
  size_t heap_size = KERNEL_HEAP_SIZE + (sizeof(page) * pages_cnt) + (sizeof(memstack) * pages_cnt);
  u32_t freemem_start;

  /* желательно приберечь память <16Mb для других целей */
  offs_t heap_start;
  if((low_freemem_start + heap_size + MIN_FREE_MEMORY) >= memory_size){
    /* памяти не хватит!! suxx */
    while(1);
  }

  if(memory_size <= LOWMEM_SIZE /* 16Mb */){
    heap_start = LOWMEM_SIZE - heap_size;
    freemem_start = low_freemem_start;
  } else if((memory_size - LOWMEM_SIZE) <= LOWMEM_SIZE){
    heap_start = memory_size - LOWMEM_SIZE;
    freemem_start = low_freemem_start;
  } else {
    heap_start = LOWMEM_SIZE;
    freemem_start = heap_start + heap_size;
  }

  /* хип ядра */
  HeapMemBlock *kheap;

  kheap = (HeapMemBlock *) heap_start;
  kheap->ptr = 0;
  kheap->size = heap_size / sizeof(HeapMemBlock);

  extern HeapMemBlock *heap_free_ptr;
  extern HeapMemBlock kmem_block;

  kmem_block.ptr = heap_free_ptr = &kmem_block;
  kmem_block.size = 0;
  
  free((void *)((unsigned int)kheap + sizeof(HeapMemBlock)));

  /* тут уже можно использовать оператор new */
  hal = new HAL(__mbi);
  hal->pages_cnt = pages_cnt;
  hal->phys_page = new page[hal->pages_cnt];
  hal->free_page = new memstack;
  hal->free_page->n = PAGE(freemem_start);

  for(u32_t i=0; i<hal->pages_cnt; i++)
    alloc_page(i);
  
  /* заполним пул свободных страниц страницами, лежащими ниже USER_PAGETABLE_DATA */
  for(u32_t i = PAGE(freemem_start); (i < (hal->pages_cnt - PAGE(freemem_start))) && (i < PAGE(USER_PAGETABLE_DATA)); i++){
    put_page(i);
  }

  /* создаем пул свободных нижних (<16Mb) страниц */
  if(freemem_start != low_freemem_start){
    hal->free_lowpage = new memstack;
    hal->free_lowpage->n = PAGE(low_freemem_start);
    for(u32_t i = PAGE(low_freemem_start) + 1; i < (PAGE(LOWMEM_SIZE) - PAGE(low_freemem_start)); i++){
      put_lowpage(i);
    }
  }

  kpagedir = (u32_t *) (get_page()*PAGE_SIZE);

  /* создадим таблицы страниц для всей памяти, входящей в KERNEL_MEM_LIMIT (32 каталога для 128 мегабайт) */
  for(u32_t i=0; i < KERNEL_MEM_LIMIT/(PAGE_SIZE*1024); i++){
    kpagedir[i] = (get_page()*PAGE_SIZE) | 3;
  }

  for(u32_t i=0; i < KERNEL_MEM_LIMIT/(PAGE_SIZE*1024); i++){
    map_page(PAGE(kpagedir[i]), PAGE(kpagedir[i]), kpagedir, 3); /* мапим на физический адрес */
  }
  
  /* смонтируем память от нуля до начала свободной памяти как есть */
  for(u32_t i=0; i < PAGE(low_freemem_start) ; i++){
    map_page(i, i, kpagedir, 3); /* мапим на физический адрес */
  }

  /* смонтируем heap */
  for(u32_t i=PAGE(heap_start); i < PAGE(heap_start + heap_size) ; i++){
    map_page(i, i, kpagedir, 3);
  }

  /* дополним пул свободных страниц оставшимися свободными страницами */
  for(u32_t i = PAGE(USER_PAGETABLE_DATA); i < SYSTEM_PAGES_MAX; i++){
    put_page(i);
  }
  
  enable_paging(kpagedir);
}

#if 1
void * kmalloc(register size_t size)
{
#if 0
  if (!size)
    return 0;
  mem_block_t *p, *prevp, *block;
  size_t asize;

  /* округлим объём */
  asize = size - (size % MM_MINALLOC);
  if (size % MM_MINALLOC)
    asize += MM_MINALLOC;

  prevp = 0;
  p = start_block;

  /* ищем блок подходящего размера */
  while (p->size < asize) {
    p = p->next;
    if (!p) {
      hal->panic("KMem: can't allocate %d bytes of memory!\n", asize);
    }
    prevp = p;
  }

  //printk("p=0x%X, p->size=0x%X, p->next=0x%X \n", p, p->size, p->next);

  if (p->size > asize) {	/* используем только часть блока */
    block = p;
    p = (mem_block_t *) ((u32_t) p + asize);	/* скорректируем указатель */
    p->size = block->size - asize;	/* вычтем выделяемый размер */
    p->next = block->next;
    if (prevp)
      prevp->next = p;
    else
      start_block = p;		/* если p - первый блок */
  } else {			/* при выделении используем весь блок */
    if (prevp)
      prevp->next = p->next;
    else
      start_block = p->next;
    block = p;
  }

  return block;
#endif
  return 0;
}

/*
  mem_free(): включает блок в список свободной памяти
  ptr - указатель на начало блока
  size - размер блока в байтах
*/
void kmfree(register void *ptr, register size_t size)
{
#if 0
  u32_t *pt = (u32_t *) ptr;
  u32_t end_pt = (u32_t) ptr + size; 
  for(; (u32_t)pt < end_pt; pt += sizeof(u32_t))
  *pt = 0;


  //printk("Freing 0x%X(0x%X)\n", (u32_t)ptr, (u32_t)size);
  mem_block_t *block = (mem_block_t *) ptr;
  mem_block_t *p = start_block;

  block->size = size;
  block->next = 0;

  /* Ищем место для вставки блока */
  //  while((block < p) && (p->next) && (p->next > block))
  //    printk("free: p=0x%X, p->size=%d, p->next=0x%X ", p, p->size, p->next);

  //  printk("free: p->next=0x%X ", p->next);

  while ((p < block) && (p->next) && (p->next > block)) {
    p = p->next;
  }

  if (block < start_block) {	/* Если блок оказался в самом начале списка: */
    block->next = p;
    start_block = block;
  } else {
    /* Слить block с p->next? */
    if ((p->next) && ((u32_t) p->next == ((u32_t) block + block->size))) {
      block->size += p->next->size;
      block->next = p->next->next;
      //      if(p->next == end_block) end_block = block;
      p->next = block;
    }

    /* Слить p и block? */
    if (((u32_t) p + p->size) == (u32_t) block) {
      p->size += block->size;
      /* Если block был слит с p->next */
      if (p->next == block)
	p->next = block->next;
      block = p;
    }

    /* Если ничего не было слито, просто вставляем блок */
    if ((block != p) && (p->next != block)) {
      block->next = p->next;
      p->next = block;
    }
  }
#endif
}
#endif

