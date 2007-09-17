#include "alloc.h"
#include <stdlib.h>

void *jvm_malloc(int size) {
	return malloc(size);
}

void  jvm_free(void *ptr) {
	free(ptr);
}
