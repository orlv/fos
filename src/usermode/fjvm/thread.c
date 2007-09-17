#include "thread.h"
#include "exec.h"
#include "alloc.h"

extern int my_tid();

#define STACK_SIZE 1024 * 4 // 4 KB stack
#define HEAP_SIZE  1024 * 1024 // 1mb heap

static JavaThread_t *java_threads = NULL;

void java_thread_init() {
	java_threads = (JavaThread_t*)jvm_malloc(sizeof(JavaThread_t));
	
	java_threads->parent = NULL;
	java_threads->next = NULL;
	
	java_threads->fos_tid = my_tid();
	
	java_threads->thread = NULL;
	
	java_threads->exec = exec_create(HEAP_SIZE);
	java_threads->exec->stack = (int*)jvm_malloc(STACK_SIZE);
	java_threads->exec->sp = java_threads->exec->stack;
}

JavaThread_t *java_thread_current() {
	int tid = my_tid();
	JavaThread_t *thread = java_threads;
	
	while (thread) {
		if (tid == thread->fos_tid) {
			return thread;
		}
		
		thread = thread->next;
	}
	
	return NULL;
}

JavaThread_t *java_thread_all() {
	return java_threads;
}

int	java_thread_getfostid(Object_t *java_thread) {
	JavaThread_t *thread = java_threads;
	
	while (thread) {
		if (thread->thread == java_thread) {
			return thread->fos_tid;
		}
		
		thread = thread->next;
	}
	
	return 0;
}

