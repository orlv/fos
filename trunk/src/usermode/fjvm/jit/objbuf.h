#ifndef OBJBUF_H_
#define OBJBUF_H_

typedef
struct ObjBuffer_s {
	unsigned char *buffer;
	
	unsigned int   size;
	unsigned int   offset;
} ObjBuffer_t;

ObjBuffer_t * objbuf_create(unsigned int size);

#endif /*OBJBUF_H_*/
