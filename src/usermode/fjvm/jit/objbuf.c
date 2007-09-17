#include "objbuf.h"
#include "../alloc.h"

ObjBuffer_t * objbuf_create(unsigned int size) {
	ObjBuffer_t *buf = jvm_malloc(sizeof(ObjBuffer_t));
	
	buf->offset = 0;
	buf->size = size;
	
	buf->buffer = jvm_malloc(size);
	
	return buf;
}