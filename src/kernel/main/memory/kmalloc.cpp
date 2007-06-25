/*
 * kernel/memory/kmalloc.cpp
 * Copyright (C) 2007 Oleg Fedorov
 */

#include <mm.h>
#include <multiboot.h>
#include <paging.h>
#include <stdio.h>
#include <hal.h>
#include <stack.h>

void put_page(u32_t page)
{
  if(!page_status(page) || !free_page(page)){ /* если эта страница больше никем не используется */
    hal->free_page->push(page);
    atomic_inc(&hal->free_pages);
  }
}

u32_t get_page()
{
  if(atomic_read(&hal->free_pages)){
    atomic_dec(&hal->free_pages);
    return hal->free_page->pop();
  } else {
    return 0;
  }
}

void put_lowpage(u32_t page)
{
  if(!page_status(page) || !free_page(page)){ /* если эта страница больше никем не используется */
    hal->free_lowpage->push(page);
    atomic_inc(&hal->free_lowpages);
  }
}

u32_t get_lowpage()
{
  atomic_dec(&hal->free_lowpages);
  return hal->free_lowpage->pop();
}


#define KERNEL_HEAP_SIZE 0x400000 /* 4Mb */
#define LOWMEM_SIZE 0x1000000 /* 16Mb*/
#define MIN_FREE_MEMORY 0x400000

void free(register void *ptr);

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
    while(1) asm("incb 0xb8000+158\n" "movb $0x5f,0xb8000+159");
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
  hal->free_page = new Stack<u32_t>(pages_cnt-PAGE(freemem_start)); //memstack;
  
  atomic_set(&hal->free_pages, 0);
  
  /* заполним пул свободных страниц страницами, лежащими ниже KERNEL_MEM_LIMIT */
  for(u32_t i = PAGE(freemem_start); (i < hal->pages_cnt) && (i < PAGE(KERNEL_MEM_LIMIT)); i++){
    put_page(i);
  }

  /* создаем пул свободных нижних (<16Mb) страниц */
  if(freemem_start != low_freemem_start){
    hal->free_lowpage = new Stack<u32_t>(PAGE(LOWMEM_SIZE)-PAGE(low_freemem_start)); //memstack;
    for(u32_t i = PAGE(low_freemem_start); (i < hal->pages_cnt) && (i < PAGE(LOWMEM_SIZE)); i++){
      put_lowpage(i);
    }
  }

  hal->kmem = new Memory(0, KERNEL_MEM_LIMIT, MMU_PAGE_PRESENT|MMU_PAGE_WRITE_ACCESS);
  hal->kmem->pagedir = (u32_t *) (get_page() * PAGE_SIZE);
  kmem_set_log_addr(PAGE((u32_t)hal->kmem->pagedir), PAGE((u32_t)hal->kmem->pagedir));

  /* создадим таблицы страниц для всей памяти, входящей в KERNEL_MEM_LIMIT (32 каталога для 128 мегабайт) */
  for(u32_t i=0; i < KERNEL_MEM_LIMIT/(PAGE_SIZE*1024); i++){
    hal->kmem->pagedir[i] = (get_page()*PAGE_SIZE) | 3;
    kmem_set_log_addr(PAGE(hal->kmem->pagedir[i]), PAGE(hal->kmem->pagedir[i]));
  }

  for(u32_t i=0; i < KERNEL_MEM_LIMIT/(PAGE_SIZE*1024); i++){
    hal->kmem->mmap((void *)(hal->kmem->pagedir[i] & 0xfffff000), (void *)(hal->kmem->pagedir[i] & 0xfffff000), PAGE_SIZE);
  }

  hal->kmem->mmap((void *)hal->kmem->pagedir, (void *)hal->kmem->pagedir, PAGE_SIZE);

  /* смонтируем память от нуля до начала свободной памяти как есть */
  hal->kmem->mmap(0, 0, low_freemem_start);

  /* смонтируем heap */
  hal->kmem->mmap((void *)heap_start, (void *)heap_start, heap_size);

  /* дополним пул свободных страниц оставшимися свободными страницами */
  for(u32_t i = PAGE(KERNEL_MEM_LIMIT); i < hal->pages_cnt; i++){
    alloc_page(i);
    put_page(i);
  }

  enable_paging(hal->kmem->pagedir);
}


void *kmalloc(register size_t size)
{
  void *ptr;
  hal->mt_disable();
  if(!(ptr = hal->kmem->mem_alloc(size)))
    hal->panic("Can't allocate 0x%X bytes of kernel memory!", size);
  hal->mt_enable();

  memset(ptr, 0, size);
  return ptr;
}

void kfree(register void *ptr)
{
  hal->mt_disable();
  hal->kmem->mem_free(ptr);
  hal->mt_enable();
}

