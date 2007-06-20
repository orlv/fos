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

#include <drivers/char/tty/tty.h>
#include <mm.h>
#include <paging.h>
#include <drivers/block/vga/vga.h>
#include <string.h>
#include <system.h>
#include <stdio.h>
#include <drivers/char/timer/timer.h>
#include <hal.h>
#include <traps.h>
#include <vsprintf.h>
#include <stdarg.h>
#include <drivers/fs/modulefs/modulefs.h>
#include <fs.h>


void put_page(u32_t page)
{
  if(!page_status(page) || !free_page(page)){ /* если эта страница больше никем не используется */
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
  if(!page_status(page) || !free_page(page)){ /* если эта страница больше никем не используется */
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

Memory *kmem;

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

  /* ----------------- */
#if 0
  hal->cli();
  hal->pic = new PIC;
  hal->pic->remap(0x20, 0x28);

  int i;
  for(i = 0; i < 16; i++)
    hal->pic->mask(i);
  
  hal->gdt = new GDT;
  hal->idt = new IDT;

  setup_idt();
  hal->sti();
  
  VGA *con = new VGA;
  TTY *tty1 = new TTY(80, 25);

  tty1->stdout = con;
  tty1->SetTextColor(WHITE);

  extern TTY *stdout;
  stdout = tty1;
#endif
  /* ----------------- */
  //printk("test");
  //while(1);
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

  kmem = new Memory(0, KERNEL_MEM_LIMIT, MMU_PAGE_PRESENT|MMU_PAGE_WRITE_ACCESS);
  kmem->pagedir = (u32_t *) (get_page()*PAGE_SIZE);
  
  /* создадим таблицы страниц для всей памяти, входящей в KERNEL_MEM_LIMIT (32 каталога для 128 мегабайт) */
  for(u32_t i=0; i < KERNEL_MEM_LIMIT/(PAGE_SIZE*1024); i++){
    kmem->pagedir[i] = (get_page()*PAGE_SIZE) | 3;
  }

  for(u32_t i=0; i < KERNEL_MEM_LIMIT/(PAGE_SIZE*1024); i++){
    kmem->mmap((void *)(kmem->pagedir[i] & 0xfffff000), (void *)(kmem->pagedir[i] & 0xfffff000), PAGE_SIZE);
  }

  //kmem->mount_page(PAGE((u32_t)kmem->pagedir), PAGE((u32_t)kmem->pagedir));
  kmem->mmap((void *)kmem->pagedir, (void *)kmem->pagedir, PAGE_SIZE);
  /* смонтируем память от нуля до начала свободной памяти как есть */
  kmem->mmap(0, 0, low_freemem_start);

  /* смонтируем heap */
  kmem->mmap((void *)heap_start, (void *)heap_start, heap_size);

  /* дополним пул свободных страниц оставшимися свободными страницами */
  for(u32_t i = PAGE(USER_PAGETABLE_DATA); i < hal->pages_cnt; i++){
    alloc_page(i);
    put_page(i);
  }

  //printk("foooooo");
  //while(1);
  
  enable_paging(kmem->pagedir);
}


void *kmalloc(register size_t size)
{
  extern Memory *kmem;
  return kmem->mem_alloc(size);
}

void kfree(register void *ptr)
{
  extern Memory *kmem;
  return kmem->mem_free(ptr);
}

