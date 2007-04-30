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

Heap SystemHeap;

Heap::Heap()
{
  free_ptr = NULL;
}

Heap::~Heap()
{

}

void *Heap::malloc(size_t size)
{
  if (!size)
    return 0;
  //  printk("[%d]",size);
  HeapMemBlock *p, *prevp;
  unsigned int nunits;
  unsigned int i;

  nunits = (size + sizeof(HeapMemBlock) - 1) / sizeof(HeapMemBlock) + 1;
  if ((prevp = free_ptr) == NULL) {	/* списка своб. памяти ещё нет */
    kmem_block.ptr = free_ptr = prevp = &kmem_block;
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
      free_ptr = prevp;

      p = (HeapMemBlock *) ((unsigned long)p + sizeof(HeapMemBlock));
      /* очистим выделяемую область памяти */
      for (i = 0; i < size / sizeof(unsigned long); i++)
	*(unsigned long *)((unsigned long)p + i * sizeof(unsigned long)) = 0;

      return (void *)p;
    }

    if (p == free_ptr)		/* прошли первый цикл по списку */
      if ((p = morecore(nunits * sizeof(HeapMemBlock))) == NULL) {
	hal->halt();
	return NULL;		/* больше памяти нет */
      }
  }
}

#define NALLOC PAGE_SIZE	/* миним. число единиц памяти для запроса */

/* morecore: запрашивает у системы дополнительную память */
HeapMemBlock *Heap::morecore(unsigned int nu)
{
  char *cp;
  HeapMemBlock *up;
  if (nu < NALLOC)
    nu = 1;
  else if (nu % NALLOC)
    nu = nu / NALLOC + 1;
  cp = (char *)kmalloc(nu * PAGE_SIZE);
  if (!cp)			/* больше памяти нет */
    return NULL;
  up = (HeapMemBlock *) cp;
  up->size = (nu * PAGE_SIZE) / sizeof(HeapMemBlock);
  free((void *)((unsigned int)up + sizeof(HeapMemBlock)));
  return free_ptr;
}

/* free: включает блок в список свободной памяти */
void Heap::free(void *ptr)
{
  if (!ptr)
    return;

  HeapMemBlock *bp, *p;
  /* указатель на начало блока */
  bp = (HeapMemBlock *) ((unsigned int)ptr - sizeof(HeapMemBlock));
  for (p = free_ptr; !(bp > p && bp < p->ptr); p = p->ptr)
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
  free_ptr = p;
}

void *Heap::realloc(void *ptr, size_t size)
{
  unsigned long *dst;
  unsigned int i, oldsize;
  dst = (unsigned long *)malloc(size);
  if (!dst)
    return NULL;
  oldsize =
      ((HeapMemBlock *) ((int)ptr -
			 sizeof(HeapMemBlock)))->size * sizeof(HeapMemBlock);
  for (i = 0; i < oldsize / 4; i++)
    dst[i] = ((unsigned long *)ptr)[i];
  free(ptr);

  return (void *)dst;
}

void *operator  new(unsigned int size)
{
  //  printk("{%d}",size);
  return SystemHeap.malloc(size);
};

void *operator  new[] (unsigned int size) {
  return SystemHeap.malloc(size);
};

void *operator  new(unsigned int size, void *ptr)
{
  size = size;
  return ptr;
}

void *operator  new[] (unsigned int size, void *ptr) {
  size = size;
  return ptr;
}

void operator  delete(void *ptr)
{
  SystemHeap.free(ptr);
}

void operator  delete[] (void *ptr) {
  SystemHeap.free(ptr);
}
