/*
 * kernel/memory/kmalloc.cpp
 * Copyright (C) 2007 Oleg Fedorov
 */

#include <fos/fos.h>
//#include <c++/stack.h>

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
    system->free_pages_DMA16++;
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
