/*
 * kernel/memory/heap.cpp
 * Copyright (C) 2006-2007 Oleg Fedorov
 *
 * malloc основан на примере из книги "Язык программирования Си" (C) Б.Керниган, Д.Ритчи
 * :)
 */

#include <fos/mm.h>
#include <fos/printk.h>
#include <fos/hal.h>
#include <c++/tmutex.h>
#include <string.h>

HeapMemBlock * volatile heap_free_ptr = NULL;
HeapMemBlock kmem_block;
static HeapMemBlock *morecore(register size_t size);
void free(register void *ptr);

static TMutex heap_mutex;

static void * volatile reserved_block = NULL;

size_t volatile heap_free = 0;

atomic_t mt_state;

void *malloc(register size_t size)
{
  //printk("[0x%X] ", size);
   
  if (!size)
    return 0;

  HeapMemBlock *p, *prevp;
  unsigned int nunits;
  //iii++;
  nunits = (size + sizeof(HeapMemBlock) - 1) / sizeof(HeapMemBlock) + 1;
  //printk("{0x%X}", nunits*sizeof(HeapMemBlock));
  //heap_mutex.lock();
  __mt_disable();
  //printk("\"0x%X\"", heap_free_ptr);
  if ((prevp = heap_free_ptr) == NULL) {	/* списка своб. памяти ещё нет */
    kmem_block.next = heap_free_ptr = prevp = &kmem_block;
    kmem_block.size = 0;
  }

  for(p = prevp->next;; prevp = p, p = p->next){
    //printk("(0x%x - 0x%x -> 0x%X)", p, p->size*sizeof(HeapMemBlock), p->next);
    //if(iii > 22) while(1);
    if (p->size >= nunits){ /* достаточно большой */
      if (p->size == nunits) /* точно нужного размера */
	prevp->next = p->next;
      else { /* отрезаем хвостовую часть */
	p->size -= nunits;
	p += p->size;
	p->size = nunits;
      }
      heap_free_ptr = prevp;
      //printk("(0x%x, 0x%X)=0x%X\n", p, p->size*sizeof(HeapMemBlock), (u32_t)(p + 1));
      //heap_mutex.unlock();
      heap_free -= p->size*sizeof(HeapMemBlock);
      __mt_enable();
      memset((void *)(p+1), 0, size);
      return (void *)(p+1);
    }

    if(p == heap_free_ptr){	/* прошли первый цикл по списку */
      //heap_mutex.unlock();
      //__mt_enable();
      if(!(p = morecore(nunits*sizeof(struct HeapMemBlock)))){
	hal->panic("no free memory available in kernel heap!");
      }
    }
  }
}

void * heap_create_reserved_block()
{
  //if(!reserved_block){
    reserved_block = kmalloc(HEAP_RESERVED_BLOCK_SIZE);
    //}
  return reserved_block;
}

#define MINALLOC PAGE_SIZE	/* миним. число единиц памяти для запроса */

/* morecore: запрашивает у системы дополнительную память */
static HeapMemBlock *morecore(register size_t size)
{
  HeapMemBlock *up;
  /* используем зарезервированный блок */
  up = (HeapMemBlock *) reserved_block;
  up->size = (HEAP_RESERVED_BLOCK_SIZE) / sizeof(HeapMemBlock);
  free((void *)((unsigned int)up + sizeof(HeapMemBlock)));
  /* теперь у нас есть память в хипе для работы kmalloc() */

  /* сразу же зарезервируем новый блок */
  reserved_block = kmalloc(HEAP_RESERVED_BLOCK_SIZE);
  
  if (size < MINALLOC)
    size = MINALLOC;
  else if (size % MINALLOC)
    size = (size/MINALLOC + 1)*MINALLOC;
  
  if(!(up = (HeapMemBlock *) kmalloc(size))){
    /* Упс, больше нет свободной памяти! */
    return NULL;
  }

  up->size = size / sizeof(HeapMemBlock);
  //heap_mutex.lock(); /* не забыть снять эту блокировку внутри malloc() */

  free((void *)((unsigned int)up + sizeof(HeapMemBlock)));
  return heap_free_ptr;
}

/* free: включает блок в список свободной памяти */
void free(register void *ptr)
{
  //printk("[0x%x]", ptr);
  if (!ptr) return;

  HeapMemBlock *bp, *p;
  bp = (HeapMemBlock *)ptr - 1; /* указатель на начало блока */

  //printk("[0x%x, 0x%x]\n", bp, bp->size*sizeof(HeapMemBlock));
  
  __mt_disable();
  heap_free += bp->size*sizeof(HeapMemBlock);
  for (p = heap_free_ptr; !(p < bp && p->next > bp); p = p->next)
    if ((p >= p->next && (p < bp || p->next > bp)))
      break;			/* освобождаем блок в начале или в конце */

  /* слить с верхним соседом */
  if (bp + bp->size == p->next) {
    bp->size += p->next->size;
    bp->next = p->next->next;
  } else
    bp->next = p->next;

  /* слить с нижним соседом */
  if (p + p->size == bp) {
    p->size += bp->size;
    p->next = bp->next;
  } else
    p->next = bp;

  heap_free_ptr = p;
  
  __mt_enable();
}

void *realloc(register void *ptr, register size_t size)
{
  unsigned long *dst;
  unsigned long oldsize;
  if (!(dst = (unsigned long *)malloc(size)))
    return NULL;
  oldsize = ((HeapMemBlock *) ((unsigned long)ptr - sizeof(HeapMemBlock)))->size * sizeof(HeapMemBlock);
  memcpy(dst, ptr, oldsize);
  free(ptr);

  return (void *)dst;
}

void *operator  new(unsigned int size)
{
  return malloc(size);
}

void *operator  new[] (unsigned int size)
{
  return malloc(size);
}

void operator  delete(void *ptr)
{
  free(ptr);
}

void operator  delete[] (void *ptr)
{
  free(ptr);
}
