#define VERBOSE 1
#define FOS 1
#define JAR 1

#include <stdio.h>
#include "metadata/class.h"
#include "thread.h"
#include "method.h"

#ifndef asmlinkage
#define asmlinkage
#endif
 
asmlinkage int main() {
	printf("FJVM starting...\n");
	
	#if VERBOSE
	printf("Java threading init...\n");
	#endif

	java_thread_init();
	
	#if VERBOSE
	printf("Done\n");
	#endif
	
	Class_t *class = class_resolve("knott/Main");
	MethodInfo_t *method = method_resolve(class, utf8_hash("main") ^ utf8_hash("()V"));
	if (!method) {
		method = method_resolve(class, utf8_hash("main") ^ utf8_hash("()I"));
		
		Exec_t *exec = exec_get_current();
		printf("error code: %i", POP_INT());
		
		if (!method) {
			printf("Cannot resolve main method\n");
			return 1;	 
		}
	} else {
		method_invoke(class, NULL, method);
	}
	
	return 0;
}
