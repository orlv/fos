/*
 * kernel/memory/kmalloc.cpp
 * Copyright (C) 2005-2006 Oleg Fedorov
 */

#include <mm.h>
#include <multiboot.h>
#include <paging.h>
#include <stdio.h>
#include <hal.h>

//#define       DEBUG_KMEMORY 0
//#define DEBUG_UMEMORY 0
//#define DEBUG_MOUNT_MEMORY 0

void init_alloc(void *ptr, size_t size);

size_t memory_size;		/* Размер памяти (в Kb) */
size_t memory_used;

//#define FREE_MEM_START 0x13000

void init_memory()
{
  extern multiboot_info_t *__mbi;

  memory_size = __mbi->mem_upper + 1024;
  offs_t mods_end_ptr =
    (((module_t *) __mbi->mods_addr)[__mbi->mods_count - 1].mod_end);
  if (mods_end_ptr & 0xfff) {
    mods_end_ptr += 0x1000;
    mods_end_ptr &= 0xfffff000;
  }

  init_alloc((void *)mods_end_ptr, memory_size * 1024 - mods_end_ptr);
  //  mem_free((void *)FREE_MEM_START, 0xA0000 - FREE_MEM_START);

  setup_paging();
  //DTman = new DTMan;
  /*  printk("Memory: %dK (%dK kernel, %dK ramdisk)\n",
     memory_size,
     bi.ksize*4,
     (bi.rd_end-bi.rd_start)/1024); */
}

/* ------------------------------------------------------------------------------------- */
typedef struct mem_block_t {
  mem_block_t *next;
  size_t size;
};

static mem_block_t *start_block;

void init_alloc(void *ptr, size_t size)
{
  start_block = (mem_block_t *) ptr;
  start_block->size = size;
  start_block->next = 0;
}

void *kmalloc(size_t size)
{
  //printk("\n{ [%d]",size);
  if (!size)
    return 0;
  mem_block_t *p, *prevp, *block;
  size_t asize;

  /* округлим объём */
  asize = size - (size % MM_MINALLOC);
  if (size % MM_MINALLOC)
    asize += MM_MINALLOC;

  //printk("->[0x%X]",asize);

  prevp = 0;
  p = start_block;

  /* ищем блок подходящего размера */
  while (p->size < asize) {
    p = p->next;
    if (!p) {
      printk("KMem: can't allocate %d bytes of memory!\n", asize);
      return 0;
    }
    prevp = p;
  }

  //printk("p=0x%X, p->size=0x%X, p->next=0x%X }\n", p, p->size, p->next);

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
}

/*
  mem_free(): включает блок в список свободной памяти
  ptr - указатель на начало блока
  size - размер блока в байтах
*/
void kmfree(void *ptr, size_t size)
{
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
}
