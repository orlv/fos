#ifndef HEAP_H_
#define HEAP_H_

/*
 * Memory heap de/allocation
 */
 
#include "metadata/class.h"
#include "types.h"
#include "synch.h"
 
#define GC_WHITE 0x00000001
#define GC_GREY  0x00000002
#define GC_BlACK 0x00000004
#define GC_TEST  0x00000010
 
#define GC_GENERIC  0x00000000
#define GC_OBJECT   0x08000000
#define GC_DATA     0x04000000
 
/*
 * Single item presented on heap.
 */
typedef
struct HeapItem_s {
	void       *data;
	size_t      size;
	int			flags;
	struct HeapItem_s *next; 
} HeapItem_t;
 
/*
 * This is a heap, that contains all allocated objects.
 */
typedef
struct Heap_s {
 	size_t		size;
 	size_t		allocated;
 	
 	HeapItem_t *first;
 	HeapItem_t *last;
 	
 	count_t	    count;
 	mutex_t     mutex;
} Heap_t;
 
Heap_t * heap_create(size_t size);
void   * heap_alloc(Heap_t *heap, size_t size, int flags);
void     heap_free(Heap_t *heap, void *ptr);
void     heap_delete(Heap_t *heap);
void     heap_mark(Heap_t *heap, void *ptr, int mark);
 
HeapItem_t * heap_find(Heap_t *heap, void *ptr);
 

#endif /*HEAP_H_*/
