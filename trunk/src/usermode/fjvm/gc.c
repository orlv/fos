#include "thread.h"
#include "gc.h"
#include "alloc.h"

#include <stdio.h>

void gc_mark(Heap_t *heap, int flag, int except, int color) {
	HeapItem_t *item = heap->first;
	
	while (item != NULL) {
		if ( !(item->flags & GC_BlACK) && !(item->flags & except)) {
			if (item->flags & flag) {
				item->flags &= ~flag;
				item->flags &= 0xFFFFFFF0;
				GC_MARK(item, color);
			}
		}
		
		item = item->next;
	}
}

void gc_sweep(Heap_t *heap, int flag) {
	printf("gc_sweep(%x, %x)", (unsigned int)heap, flag);
	
	if (heap == NULL) {
		return;
	}
	
	HeapItem_t *item = heap->first;
	while (item->next) {
		if (item->next->flags & flag) {
			HeapItem_t *tmp = item->next;
			item->next = tmp->next;
			
			if (heap->last == tmp) {
				heap->last = item;
			}
			
			heap->allocated -= tmp->size;
			heap->count--;
			
			if (tmp->flags & GC_OBJECT) {
				synch_delete(&(((Object_t*)(tmp->data))->mutex));
			}
			
			jvm_free(tmp->data);
			jvm_free(tmp);
		} else {
			item = item->next;
		}
	}
	
	
}

void gc_pass_object_item(Heap_t *heap, HeapItem_t *item) {
	if (item != NULL && (item->flags & GC_GREY))
		return;
	
	GC_MARK(item, GC_GREY);
	
	Object_t *object = (Object_t *)item->data;
	HeapItem_t *data = heap_find(heap, object->data);
	
	if (data != NULL) {
		if (data->flags & GC_GREY) return;
		
		GC_MARK(data, GC_GREY);
		
		int f;
		for (f = 0; f < object->class->field_count; f++) {
			FieldInfo_t *field_info = &(object->class->fields[f]);
			
			if (field_info->signature[0] == '[' || field_info->signature[0] == 'L') {
				data = heap_find(heap, (void*)*((int*)(object->data + field_info->offset)));
				
				if (data) {
					gc_pass_object_item(heap, data);
				}
			}
		}
	}
}

void gc_pass_object(Heap_t *heap, Object_t *object) {
	HeapItem_t *item = heap_find(heap, object);
	gc_pass_object_item(heap, item);
}

void gc_pass() {
	Exec_t *exec = exec_get_current();
	Heap_t *heap = exec->heap;
	
	HeapItem_t *item = NULL;
	
	//unsigned int flags;
	//int i, f;
	
	gc_mark(heap, GC_GREY, GC_TEST, GC_WHITE);
	
	/*
	for (i = 0; i < exec->classes->count; i++) {
		hash_entry_t *entry = &(exec->classes->entries[i]);
		Class_t *class = (Class_t *)(entry->data);
		
		for (f = 0; f < class->field_count; f++) {
			FieldInfo_t *field = &(class->fields[f]);
			
			if (field->access_flags & ACC_STATIC) {
				if (field->signature[0] == '[' || field->signature[0] == 'L') {
					item = heap_find(heap, (void*)field->static_value);
					
					if (item != NULL) {
						if (item->flags & GC_BlACK) { 
						} else {
							if (item->flags & GC_OBJECT) {
								gc_pass_object_item(heap, item);
							} else {
								GC_MARK(item, GC_GREY);
							}
						}
					}
				}
			}
		}
	}
	*/
	
	JavaThread_t *threads = (JavaThread_t *)java_thread_all();
	while (threads) {
		exec = threads->exec;
		
		if (threads->thread) {
			gc_pass_object(heap, threads->thread);
		}
		
		if (exec->exception) {
			gc_pass_object(heap, exec->exception);
		}
		
		int *sp = exec->sp + 16;
		while (sp >= exec->stack) {
			item = heap_find(heap, (int*)sp);
			if (item != NULL) {
				if (!(item->flags & GC_BlACK)) {
					if (item->flags & GC_OBJECT) {
						gc_pass_object_item(heap, item);
					}
				}
			}
			
			sp--;
		}
		
		Frame_t *frame = exec->frame;
		while (frame) {
			if (!frame->method) {
				frame = frame->prev;
				continue;
			}
			
			int nlocals = frame->method->max_locals;
			int i;
			
			for (i = 0; i < nlocals; i++) {
				item = heap_find(heap, (int*)frame->locals[i]);
				
				if (item) {
					if (!(item->flags & GC_BlACK)) {
						if (item->flags & GC_OBJECT) {
							gc_pass_object_item(heap, item);
						} else {
							GC_MARK(item, GC_GREY);
						}
					}
				}
			}
			
			frame = frame->prev;
		}
		
		threads = threads->next;
	}
}
