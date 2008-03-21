/*
 * kernel/memory/kmalloc.cpp
 * Copyright (C) 2007 Oleg Fedorov
 */

#include <fos/mm.h>
#include <fos/mmu.h>
#include <fos/printk.h>
#include <fos/system.h>
#include <fos/pager.h>
#include <c++/stack.h>
#include <multiboot.h>
#include <string.h>
#include <stdlib.h>

#include <fos/drivers/tty/tty.h>

void put_page(u32_t page)
{
  if(page >= PAGE(DMA16_MEM_SIZE)){
    if(!free_page(page)){ /* если эта страница больше никем не используется */
      system->free_page->push(page);
      system->free_pages++;
    }
  } else {
    put_page_DMA16(page);
  }
}

/* если в пуле есть страницы - возвращаем одну, иначе пытаемся возвратить страницу из нижней памяти */
u32_t get_page()
{
  if(system->free_pages){
    system->free_pages--;
    return system->free_page->pop();
  } else {
    return get_page_DMA16();
  }
}

void put_page_DMA16(u32_t page)
{
  if(!free_page(page)){ /* если эта страница больше никем не используется */
    system->free_page_DMA16->push(page);
    system->free_pages_DMA16--;
  }
}

u32_t get_page_DMA16()
{
  if(system->free_pages_DMA16){
    system->free_pages_DMA16--;
    return system->free_page_DMA16->pop();
  } else {
    return 0;
  }
}

#define MEMORY_LIMIT 536870912

void init_memory()
{
  extern multiboot_info_t *__mbi;

  size_t memory_size = (__mbi->mem_upper + 1024)*1024;
  if(memory_size > MEMORY_LIMIT)
    memory_size = MEMORY_LIMIT;

  offs_t freemem_start_DMA16 = ((module_t *) __mbi->mods_addr)[__mbi->mods_count - 1].mod_end;
  if (freemem_start_DMA16 & 0xfff) {
    freemem_start_DMA16 &= 0xfffff000;
    freemem_start_DMA16 += PAGE_SIZE;
  }

  u32_t pages_cnt = PAGE(memory_size);
  size_t heap_size = KERNEL_MIN_HEAP_SIZE + (sizeof(page) * pages_cnt) + (sizeof(memstack) * pages_cnt);
  u32_t freemem_start, freemem_end;

  offs_t heap_start;
  size_t mmu_data_size = PAGE_SIZE + ((pages_cnt/(PAGE_SIZE/4))*PAGE_SIZE + (pages_cnt%(PAGE_SIZE/4) > 1)); /* каталог страниц + таблицы страниц */
  if((freemem_start_DMA16 + heap_size + mmu_data_size + MIN_FREE_MEMORY) >= memory_size){
    /* Памяти не хватит!! Suxx */
    while(1) asm("incb 0xb8000\n" "movb $0x5f,0xb8000+1");
  }

  if((DMA16_MEM_SIZE + heap_size) >= memory_size){
    heap_start = memory_size - heap_size;
    freemem_start = freemem_end = 0;
  } else {
    heap_start = DMA16_MEM_SIZE;
    freemem_start = heap_start + heap_size;
    freemem_end = memory_size;
  }

  /* ----------------- */
  preempt_reset();
  
  /* --  Хип ядра  -- */
  HeapMemBlock *kheap;

  kheap = (HeapMemBlock *) heap_start;
  kheap->next = 0;
  kheap->size = heap_size / sizeof(HeapMemBlock);

  extern HeapMemBlock *heap_free_ptr;
  extern HeapMemBlock kmem_block;

  kmem_block.next = heap_free_ptr = &kmem_block;
  kmem_block.size = 0;
  
  free((void *)(kheap+1));

  /* ------------  Тут уже можно использовать оператор new  ------------ */
  system = new SYSTEM(__mbi);
  system->pages_cnt = pages_cnt;
  system->phys_page = new page[system->pages_cnt];

  /* Создаем пул свободных нижних (<16 Мб) страниц */
  system->free_page_DMA16 = new Stack<u32_t>(PAGE(DMA16_MEM_SIZE)-PAGE(freemem_start_DMA16));
  for(u32_t i = PAGE(freemem_start_DMA16); (i < PAGE(heap_start)) && (i < PAGE(DMA16_MEM_SIZE)); i++){
    put_page_DMA16(i);
  }

  /* -- Setup console -- */
  TTY *tty1 = new TTY(80, 25);
  tty1->set_text_color(WHITE);
  extern TTY *stdout;
  stdout = tty1;

  //printk("heap size = 0x%X (0x%X)\n", heap_size, heap_size / sizeof(HeapMemBlock));
  
  /*
    Заполним пул свободных страниц страницами, лежащими ниже KERNEL_MEM_LIMIT
    Пул будет пуст, если нет свободной памяти выше 16 Мб -- запросы get_page() будут отдавать страницы из пула free_lowpage
  */
  if(freemem_start){
    system->free_page = new Stack<u32_t>(PAGE(freemem_end)-PAGE(freemem_start));
    for(u32_t i = PAGE(freemem_start); (i < PAGE(freemem_end)) && (i < PAGE(KERNEL_MEM_LIMIT)); i++){
      put_page(i);
    }
  }

  system->kmem = new VMM(0, KERNEL_MEM_LIMIT);
  system->kmem->pager = new Pager(get_page() * PAGE_SIZE, MMU_PAGE_PRESENT|MMU_PAGE_WRITE_ACCESS); /* каталог страниц ядра */
  kmem_set_log_addr(PAGE(OFFSET(system->kmem->pager->pagedir)), PAGE(OFFSET(system->kmem->pager->pagedir)));

  /* Создадим таблицы страниц для всей памяти, входящей в KERNEL_MEM_LIMIT (32 каталога для 128 мегабайт) */
  for(u32_t i=0; i < KERNEL_MEM_LIMIT/(PAGE_SIZE*1024); i++){
    system->kmem->pager->pagedir[i] = (get_page()*PAGE_SIZE) | 3;
    kmem_set_log_addr(PAGE(system->kmem->pager->pagedir[i]), PAGE(system->kmem->pager->pagedir[i]));
  }

  for(u32_t i=0; i < KERNEL_MEM_LIMIT/(PAGE_SIZE*1024); i++){
    system->kmem->mmap(system->kmem->pager->pagedir[i] & 0xfffff000, PAGE_SIZE, MAP_FIXED, system->kmem->pager->pagedir[i] & 0xfffff000, 0);
    //if(i==2) while(1);
    //mmap(ADDRESS(system->kmem->pager->pagedir[i] & 0xfffff000), ADDRESS(system->kmem->pager->pagedir[i] & 0xfffff000), PAGE_SIZE);
  }

  system->kmem->mmap(OFFSET(system->kmem->pager->pagedir), PAGE_SIZE, MAP_FIXED, OFFSET(system->kmem->pager->pagedir), 0);
    //mmap(system->kmem->pager->pagedir, system->kmem->pager->pagedir, PAGE_SIZE);

  /* Смонтируем память от нуля до начала свободной памяти как есть */
  system->kmem->mmap(0, freemem_start_DMA16, MAP_FIXED, 0, 0);
    //mmap(0, 0, low_freemem_start);
  //while(1);  

  /* Смонтируем heap */
  system->kmem->mmap(heap_start, heap_size, MAP_FIXED, heap_start, 0);
    //mmap(ADDRESS(heap_start), ADDRESS(heap_start), heap_size);

  /* Дополним пул свободных страниц оставшимися свободными страницами (если они остались, конечно) */
  for(u32_t i = PAGE(KERNEL_MEM_LIMIT); i < system->pages_cnt; i++){
    alloc_page(i);
    put_page(i);
  }

  enable_paging(system->kmem->pager->pagedir);
  heap_create_reserved_block();
  //printk("fooo");
  //while(1);
}

void *kmalloc(register size_t size)
{
  void *ptr;
  system->preempt.disable();
  if(!(ptr = system->kmem->mmap(0, size, 0, 0, 0)))
    system->panic("No memory left!!! \nCan't allocate 0x%X bytes of kernel memory!", size);
  system->preempt.enable();

  memset(ptr, 0, size);
  return ptr;
}

void kfree(register void *ptr, size_t size)
{
  system->preempt.disable();
  system->kmem->munmap((off_t)ptr, size);
  system->preempt.enable();
}
