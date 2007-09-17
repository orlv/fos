#ifndef GC_H_
#define GC_H_

#include "exec.h"
#include "heap.h"

#define GC_MARK(item, flag) \
	if (item) { 		    \
		item->flags &= 0xFFFFFFF0; \
		item->flags |= flag; \
	}
	
void gc_mark(Heap_t *heap, int flag, int except, int color);
void gc_sweep(Heap_t *heap, int flag);
void gc_pass();

#endif /*GC_H_*/
