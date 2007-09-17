#include "alloc.h"
#include "hash.h"
#include "thread.h"
#include "exec.h"

#include <stdio.h>

Frame_t *frame_create(Class_t *class, MethodInfo_t *method) {
	Exec_t *exec = exec_get_current();
	Frame_t *frame = (Frame_t*)jvm_malloc(sizeof(Frame_t));
	
	frame->method = method;
	frame->cp = method->code;
	
	frame->locals = exec->sp - method->arg_count;
	frame->prev = exec->frame;
	
	exec->sp = frame->locals + method->max_locals;
	exec->frame = frame;
	
	return frame;
}

void frame_delete() {
	Exec_t *exec = exec_get_current();
	exec->sp = exec->frame->locals;
	
	Frame_t *prev = exec->frame->prev;
	jvm_free(exec->frame);
	
	exec->frame = prev;
}


Exec_t *exec_create(size_t heapSize) {
	Exec_t *exec = (Exec_t*)jvm_malloc(sizeof(Exec_t));
	
	exec->strings = hash_table_create(HASH_SIZE_UTF8);
	exec->classes = hash_table_create(HASH_SIZE_CLASS);
	
	exec->heap = heap_create(heapSize);
	
	exec->frame = NULL;
	exec->exception = NULL;
	
	return exec;
}

void exec_delete(Exec_t *exec) {
	hash_table_free(exec->strings);
	hash_table_free(exec->classes);
	
	heap_delete(exec->heap);
	
	jvm_free(exec); 
}

Exec_t * exec_get_current() {
	JavaThread_t *thread = java_thread_current();
	return thread->exec;
}

void    exec_rise_exception(Object_t *exception) {
	Exec_t *exec = exec_get_current();
	if (exec->exception != NULL) {
		printf("Double exeception\n");
	}
	
	exec->exception = exception;
	
	//GC
}
