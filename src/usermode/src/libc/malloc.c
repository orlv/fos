/*
 * apps/lib/heap.c
 *
 * malloc основан на примере из книги "Язык программирования Си" (C) Б.Керниган, Д.Ритчи
 * :)
 */

#include <stdlib.h>
#include <fos/page.h>
#include <string.h>

#define MM_MINALLOC getpagesize() /* размер выделяемой единицы */

struct HeapMemBlock{
  struct HeapMemBlock *ptr;
  size_t  size;
};

static struct HeapMemBlock *free_ptr=0;
static struct HeapMemBlock kmem_block;

struct HeapMemBlock * morecore(unsigned int nu);

void * malloc(size_t size)
{
  if(!size) return 0;
  //  printk("[%d]",size);
  struct HeapMemBlock *p, *prevp;
  unsigned int	nunits;

  nunits = (size + sizeof(struct HeapMemBlock) - 1) / sizeof(struct HeapMemBlock) + 1;
  prevp = free_ptr;
  if (!prevp){ /* списка своб. памяти ещё нет */
    kmem_block.ptr = free_ptr = prevp = &kmem_block;
    kmem_block.size = 0;
  }
  for(p = prevp->ptr;; prevp = p, p = p->ptr){
    if (p->size >= nunits){ /* достаточно большой */
      if (p->size == nunits) /* точно нужного размера */
	prevp->ptr = p->ptr;
      else { /* отрезаем хвостовую часть */
	p->size -= nunits;// + sizeof(HeapMemBlock);
	p += p->size;
	p->size = nunits;
      }
      free_ptr = prevp;
	 
      memset((void *)(p+1), 0, size);
      return (void *)(p+1);
    }

    if(p == free_ptr)	/* прошли первый цикл по списку */
      if((p = morecore(nunits*sizeof(struct HeapMemBlock))) == NULL){
	/* EXIT */
	return NULL;	/* больше памяти нет */
      }
  }
}

#define NALLOC PAGE_SIZE	/* миним. число единиц памяти для запроса */

/* morecore: запрашивает у системы дополнительную память */
struct HeapMemBlock * morecore(unsigned int nu)
{
  char *cp;
  struct HeapMemBlock *up;
  if (nu < NALLOC)
    nu = 1;
  else if (nu % NALLOC)
    nu = nu / NALLOC + 1;
  cp = (char *)kmalloc(nu * PAGE_SIZE, 0);
  if (!cp) /* больше памяти нет */
    return NULL;
  up = (struct HeapMemBlock *)cp;
  up->size = (nu * PAGE_SIZE)/sizeof(struct HeapMemBlock);
  free((void *)((unsigned int)up + sizeof(struct HeapMemBlock)));
  return free_ptr;
}

/* free: включает блок в список свободной памяти */
void free(void *ptr)
{
  if(!ptr)
    return;

  struct HeapMemBlock *bp, *p;
  /* указатель на начало блока */
  bp = (struct HeapMemBlock *)((unsigned int)ptr - sizeof(struct HeapMemBlock));
  for (p = free_ptr; !(bp > p && bp < p->ptr); p = p->ptr)
    if (p >= p->ptr && (bp > p || bp < p->ptr))
      break;	/* освобождаем блок в начале или в конце */
  
  if (bp + bp->size == p->ptr){ /* слить с верхним соседом */
    bp->size += p->ptr->size;
    bp->ptr = (struct HeapMemBlock *)p->ptr->ptr;
  } else
    bp->ptr = p->ptr;
  if (p + p->size == bp){ /* слить с нижним соседом */
    p->size += bp->size;
    p->ptr = bp->ptr;
  } else
    p->ptr = bp;
  free_ptr = p;
}

void * realloc(void *ptr, size_t size)
{
  unsigned long *dst;
  unsigned int i, oldsize;
  dst = (unsigned long *) malloc(size);
  if(!dst) return NULL;
  oldsize = ((struct HeapMemBlock *)((int)ptr - sizeof(struct HeapMemBlock)))->size * sizeof(struct HeapMemBlock);
  for(i=0; i<oldsize/4; i++)
    dst[i] = ((unsigned long *)ptr)[i];
  free(ptr);
 
  return (void *)dst;
}
