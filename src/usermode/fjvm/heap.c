#include "heap.h"
#include "alloc.h"
#include <stdio.h>
//#include "gc.h"

Heap_t * heap_create(size_t size) {
    Heap_t *heap = (Heap_t *)jvm_malloc(sizeof(Heap_t));
    
    if (heap == NULL)
	return NULL;
	
    heap->size = size;
    heap->allocated = 0;
    
    heap->first = NULL;
    heap->last = NULL;
    
    heap->count = 0;
    
    synch_init(&(heap->mutex));
    
    return heap;
}

void * heap_alloc(Heap_t *heap, size_t size, int flags) {
    if (heap == NULL) return NULL;
    
    if (size == 0) {
		return NULL;
    }
    
    synch_lock(&(heap->mutex));
    
    if ((heap->size - heap->allocated) < size) {
		//gc_pass();
		//gc_sweep(heap, GC_WHITE);
	
		if ((heap->size - heap->allocated) < size) {
	    	printf("HEAP: Out-of-memory\n");
	    	synch_unlock(&(heap->mutex));
	    	return NULL;
		}
    }
    
    HeapItem_t *item = jvm_malloc(sizeof(HeapItem_t));
    if (item == NULL) {
    	synch_unlock(&(heap->mutex));
    	return NULL;
    }
    
    item->data = jvm_malloc(size);
    if (item->data == NULL) {
		printf("HEAP: Not engough memory for data.\n");
		jvm_free(item);
		synch_unlock(&(heap->mutex));
	
		return NULL;
    }
    
    item->size = size;
    item->flags = flags | GC_GREY;
    item->next = NULL;
    
    if (heap->first != NULL) {
		heap->last->next = item;
		heap->last = item;	
    } else {
		heap->first = item;
		heap->last = item;
    }
    
    heap->allocated += size;
    heap->count++;
    
    synch_unlock(&(heap->mutex));
    
    return item->data;
}

void heap_free(Heap_t *heap, void* ptr) {
    if (!heap || !ptr) return;
    
    synch_lock(&(heap->mutex));
    
    HeapItem_t *item = heap->first;
    if (item->data == ptr) {
		heap->first = item->next;
		//jvm_free(item->data)
		heap->allocated -= item->size;
		heap->count--;
		jvm_free(item);
    } else {
		while (item->next != NULL) {
	    	if (item->next->data == ptr) {
				HeapItem_t *tmp = item->next;
				item->next = tmp->next;
			
				if (heap->last == tmp) 
		    		heap->last = item;
		    
				heap->allocated -= item->size;
				heap->count--;
		
				jvm_free(tmp);
				break;
	    	} else {
				item = item->next;
	    	}
		}
    }
    
    jvm_free(ptr);
    synch_unlock(&(heap->mutex));
}


void heap_delete(Heap_t *heap) {
    if (heap == NULL) return;
    
    synch_lock(&(heap->mutex));
    
    HeapItem_t *item = heap->first;
    while (item != NULL) {
		heap->first = item->next;
		jvm_free(item->data);
		jvm_free(item);
	
		item = heap->first;
    }
    
    jvm_free(heap);
    synch_unlock(&(heap->mutex));    
}

void heap_mark(Heap_t *heap, void *obj, int mark) {
    if (!heap) return;
    
    synch_lock(&(heap->mutex));
    
    HeapItem_t *item = heap->first;
    while (item) {
		if (item->data == obj) {
	    	item->flags = (item->flags & 0xFFFF0000) | mark;
	    	return;
		}
		
		item = item->next;
    }
    
    synch_unlock(&(heap->mutex));
}

HeapItem_t * heap_find(Heap_t *heap, void *obj) {
    synch_lock(&(heap->mutex));
    
    HeapItem_t *item = heap->first;
    while (item) {
		if (item->data == obj) {
	    	return item;
		}
	
		item = item->next;
    }
    
    synch_unlock(&(heap->mutex));
    return NULL;
}
