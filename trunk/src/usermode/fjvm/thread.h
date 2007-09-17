#ifndef THREAD_H_
#define THREAD_H_

#include "exec.h"
#include "metadata/class.h"

typedef struct JavaThread_s JavaThread_t;
struct JavaThread_s {
	int			fos_tid;
	Exec_t	   *exec;
	Object_t   *thread;
	
	JavaThread_t	*parent;
	JavaThread_t    *next;
};

void java_thread_init();

JavaThread_t *java_thread_current();
JavaThread_t *java_thread_all();
JavaThread_t *java_thread_create(JavaThread_t *parent, Object_t *java_thread);

int			java_thread_getfostid(Object_t *java_thread);

#endif /*THREAD_H_*/
