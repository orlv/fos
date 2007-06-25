/*
 * kernel/memory/heap.cpp
 * Copyright (C) 2006 Oleg Fedorov
 *
 * malloc был основан на примере из книги "Язык программирования Си"" (C) Б.Керниган, Д.Ритчи
 * :)
 */

#include <mm.h>
#include <stdio.h>
#include <hal.h>
#include <string.h>
#include <paging.h>

HeapMemBlock *heap_free_ptr = NULL;
HeapMemBlock kmem_block;
HeapMemBlock *morecore(register unsigned int nu);
void free(register void *ptr);

void *malloc(register size_t size)
{
  if (!size)
    return 0;

  HeapMemBlock *p, *prevp;
  unsigned int nunits;

  nunits = (size + sizeof(HeapMemBlock) - 1) / sizeof(HeapMemBlock) + 1;
  if ((prevp = heap_free_ptr) == NULL) {	/* списка своб. памяти ещё нет */
    kmem_block.ptr = heap_free_ptr = prevp = &kmem_block;
    kmem_block.size = 0;
  }
  for (p = prevp->ptr;; prevp = p, p = p->ptr) {
    if (p->size >= nunits) {	/* достаточно большой */
      if (p->size == nunits)	/* точно нужного размера */
	prevp->ptr = p->ptr;
      else {			/* отрезаем хвостовую часть */
	p->size -= nunits;	// + sizeof(HeapMemBlock);
	p += p->size;
	p->size = nunits;
      }
      heap_free_ptr = prevp;

      p = (HeapMemBlock *) ((unsigned long)p + sizeof(HeapMemBlock));
      /* очистим выделяемую область памяти */
      memset(p, 0, size);
      return (void *)p;
    }

    if (p == heap_free_ptr)		/* прошли первый цикл по списку */
      if ((p = morecore(nunits * sizeof(HeapMemBlock))) == NULL) {
	hal->halt();
	return NULL;		/* больше памяти нет */
      }
  }
}

#define NALLOC PAGE_SIZE	/* миним. число единиц памяти для запроса */

/* morecore: запрашивает у системы дополнительную память */
HeapMemBlock *morecore(register unsigned int nu)
{
  char *cp;
  HeapMemBlock *up;
  if (nu < NALLOC)
    nu = 1;
  else if (nu % NALLOC)
    nu = nu / NALLOC + 1;
  if(nu > PAGE_SIZE)
    return NULL;
  cp = (char *)kmalloc(nu * PAGE_SIZE);
  if (!cp)			/* больше памяти нет */
    return NULL;
  up = (HeapMemBlock *) cp;
  up->size = (nu * PAGE_SIZE) / sizeof(HeapMemBlock);
  free((void *)((unsigned int)up + sizeof(HeapMemBlock)));
  return heap_free_ptr;
}

/* free: включает блок в список свободной памяти */
void free(register void *ptr)
{
  if (!ptr)
    return;

  HeapMemBlock *bp, *p;
  /* указатель на начало блока */
  bp = (HeapMemBlock *) ((unsigned int)ptr - sizeof(HeapMemBlock));
  for (p = heap_free_ptr; !(bp > p && bp < p->ptr); p = p->ptr)
    if (p >= p->ptr && (bp > p || bp < p->ptr))
      break;			/* освобождаем блок в начале или в конце */

  if (bp + bp->size == p->ptr) {	/* слить с верхним соседом */
    bp->size += p->ptr->size;
    bp->ptr = (HeapMemBlock *) p->ptr->ptr;
  } else
    bp->ptr = p->ptr;
  if (p + p->size == bp) {	/* слить с нижним соседом */
    p->size += bp->size;
    p->ptr = bp->ptr;
  } else
    p->ptr = bp;
  heap_free_ptr = p;
}

void *realloc(register void *ptr, register size_t size)
{
  unsigned long *dst;
  unsigned long oldsize;
  if (!(dst = (unsigned long *)malloc(size)))
    return NULL;
  oldsize = ((HeapMemBlock *) ((unsigned long)ptr - sizeof(HeapMemBlock)))->size * sizeof(HeapMemBlock);
  __memcpy(dst, ptr, oldsize);
  free(ptr);

  return (void *)dst;
}

void *operator  new(unsigned int size)
{
  return malloc(size);
};

void *operator  new[] (unsigned int size)
{
  return malloc(size);
};

void operator  delete(void *ptr)
{
  free(ptr);
}

void operator  delete[] (void *ptr)
{
  free(ptr);
}
