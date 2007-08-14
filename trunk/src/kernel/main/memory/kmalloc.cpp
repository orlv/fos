/*
 * kernel/memory/kmalloc.cpp
 * Copyright (C) 2007 Oleg Fedorov
 */

#include <fos/mm.h>
#include <fos/mmu.h>
#include <fos/printk.h>
#include <fos/hal.h>
#include <fos/pager.h>
#include <c++/stack.h>
#include <multiboot.h>
#include <string.h>

//#include <fos/drivers/char/tty/tty.h>

void put_page(u32_t page)
{
  if(page >= PAGE(LOWMEM_SIZE)){
    if(!free_page(page)){ /* если эта страница больше никем не используется */
      hal->free_page->push(page);
      hal->free_pages.inc();
    }
  } else {
    put_lowpage(page);
  }
}

/* если в пуле есть страницы - возвращаем одну, иначе пытаемся возвратить страницу из нижней памяти */
u32_t get_page()
{
  if(hal->free_pages.read()){
    hal->free_pages.dec();
    return hal->free_page->pop();
  } else {
    return get_lowpage();
  }
}

void put_lowpage(u32_t page)
{
  if(!free_page(page)){ /* если эта страница больше никем не используется */
    hal->free_lowpage->push(page);
    hal->free_lowpages.inc();
  }
}

u32_t get_lowpage()
{
  if(hal->free_lowpages.read()){
    hal->free_lowpages.dec();
    return hal->free_lowpage->pop();
  } else {
    return 0;
  }
}

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
  size_t heap_size = KERNEL_MIN_HEAP_SIZE + (sizeof(page) * pages_cnt) + (sizeof(memstack) * pages_cnt);
  u32_t freemem_start, freemem_end;

  offs_t heap_start;
  size_t mmu_data_size = PAGE_SIZE + ((pages_cnt/(PAGE_SIZE/4))*PAGE_SIZE + (pages_cnt%(PAGE_SIZE/4) > 1)); /* каталог страниц + таблицы страниц */
  if((low_freemem_start + heap_size + mmu_data_size + MIN_FREE_MEMORY) >= memory_size){
    /* Памяти не хватит!! Suxx */
    while(1) asm("incb 0xb8000\n" "movb $0x5f,0xb8000+1");
  }

  if((LOWMEM_SIZE + heap_size) >= memory_size){
    heap_start = memory_size - heap_size;
    freemem_start = freemem_end = 0;
  } else {
    heap_start = LOWMEM_SIZE;
    freemem_start = heap_start + heap_size;
    freemem_end = memory_size;
  }

  /* ----------------- */
  __mt_reset();
  
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
  hal = new HAL(__mbi);
  hal->pages_cnt = pages_cnt;
  hal->phys_page = new page[hal->pages_cnt];

  /* Создаем пул свободных нижних (<16 Мб) страниц */
  hal->free_lowpage = new Stack<u32_t>(PAGE(LOWMEM_SIZE)-PAGE(low_freemem_start));
  for(u32_t i = PAGE(low_freemem_start); (i < PAGE(heap_start)) && (i < PAGE(LOWMEM_SIZE)); i++){
    put_lowpage(i);
  }

  /* --  --  --  -- */
  /*  TTY *tty1 = new TTY(80, 25);
  tty1->set_text_color(WHITE);
  extern TTY *stdout;
  stdout = tty1;*/
  //printk("heap size = 0x%X (0x%X)\n", heap_size, heap_size / sizeof(HeapMemBlock));
  
  /*
    Заполним пул свободных страниц страницами, лежащими ниже KERNEL_MEM_LIMIT
    Пул будет пуст, если нет свободной памяти выше 16 Мб -- запросы get_page() будут отдавать страницы из пула free_lowpage
  */
  if(freemem_start){
    hal->free_page = new Stack<u32_t>(PAGE(freemem_end)-PAGE(freemem_start));
    for(u32_t i = PAGE(freemem_start); (i < PAGE(freemem_end)) && (i < PAGE(KERNEL_MEM_LIMIT)); i++){
      put_page(i);
    }
  }

  hal->kmem = new Memory(0, KERNEL_MEM_LIMIT);
  hal->kmem->pager = new Pager(get_page() * PAGE_SIZE, MMU_PAGE_PRESENT|MMU_PAGE_WRITE_ACCESS); /* каталог страниц ядра */
  kmem_set_log_addr(PAGE(OFFSET(hal->kmem->pager->pagedir)), PAGE(OFFSET(hal->kmem->pager->pagedir)));

  /* Создадим таблицы страниц для всей памяти, входящей в KERNEL_MEM_LIMIT (32 каталога для 128 мегабайт) */
  for(u32_t i=0; i < KERNEL_MEM_LIMIT/(PAGE_SIZE*1024); i++){
    hal->kmem->pager->pagedir[i] = (get_page()*PAGE_SIZE) | 3;
    kmem_set_log_addr(PAGE(hal->kmem->pager->pagedir[i]), PAGE(hal->kmem->pager->pagedir[i]));
  }

  for(u32_t i=0; i < KERNEL_MEM_LIMIT/(PAGE_SIZE*1024); i++){
    hal->kmem->mmap(ADDRESS(hal->kmem->pager->pagedir[i] & 0xfffff000), ADDRESS(hal->kmem->pager->pagedir[i] & 0xfffff000), PAGE_SIZE);
  }

  hal->kmem->mmap(hal->kmem->pager->pagedir, hal->kmem->pager->pagedir, PAGE_SIZE);

  /* Смонтируем память от нуля до начала свободной памяти как есть */
  hal->kmem->mmap(0, 0, low_freemem_start);

  /* Смонтируем heap */
  hal->kmem->mmap(ADDRESS(heap_start), ADDRESS(heap_start), heap_size);

  /* Дополним пул свободных страниц оставшимися свободными страницами (если они остались, конечно) */
  for(u32_t i = PAGE(KERNEL_MEM_LIMIT); i < hal->pages_cnt; i++){
    alloc_page(i);
    put_page(i);
  }

  enable_paging(hal->kmem->pager->pagedir);
  heap_create_reserved_block();
}


void *kmalloc(register size_t size)
{
  void *ptr;
  hal->mt_disable();
  if(!(ptr = hal->kmem->mem_alloc(size)))
    hal->panic("No memory left!!! \nCan't allocate 0x%X bytes of kernel memory!", size);
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

