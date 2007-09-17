#include "constpool.h"
#include "constobject.h"

#include <stdio.h>
#include <string.h>

#include "../../alloc.h"
#include "../../types.h"


ConstPool_t * constpool_create(int count) {
	ConstPool_t *pool = (ConstPool_t *)jvm_malloc(
		sizeof(ConstPool_t));
		
	pool->count = count;
	pool->pool = (ConstObject_t**)jvm_malloc(
		sizeof(ConstObject_t *) * count);
	
	return pool;
}

void constpool_delete(ConstPool_t *pool) {
	int i;
	
	for (i = 0; i < pool->count; i++) {
		jvm_free(pool->pool[i]);
	}
	
	jvm_free(pool->pool);
	jvm_free(pool);
}

int constpool_length(ConstPool_t *pool) {
	return pool->count;
}

double constpool_get_double(ConstPool_t *pool, int index) {
	return ((ConstDouble_t *)pool->pool[index]->data)->value;
}

long long constpool_get_long(ConstPool_t *pool, int index) {
	return ((ConstLong_t *)pool->pool[index]->data)->value;
}

float constpool_get_float(ConstPool_t *pool, int index) {
	return ((ConstSingle_t *)pool->pool[index]->data)->value;
}

int constpool_get_integer(ConstPool_t *pool, int index) {
	return ((ConstInteger_t *)pool->pool[index]->data)->value;
}

ConstSign_t * constpool_get_signature(ConstPool_t *pool, int index) {
	return (ConstSign_t *)pool->pool[index]->data;
}

ConstClass_t *constpool_get_class(ConstPool_t *pool, int index) {
	return (ConstClass_t *)pool->pool[index]->data;
}

char * constpool_get_string(ConstPool_t *pool, int index) {
	int id;
	
	int tag = pool->pool[index]->tag;
	//printf("constpool_get_string(%x, %i); tag = %i\n", pool, index, tag);
	
	if (tag == CONST_STRING) {;
		id = ((ConstString_t *)pool->pool[index]->data)->stringID;
		return ((ConstUTF8_t *)pool->pool[id]->data)->data;
	} else {
		return ((ConstUTF8_t *)pool->pool[index]->data)->data;
	}
}

ConstMethodref_t * constpool_get_method(ConstPool_t *pool, int index) {
	return (ConstMethodref_t *)pool->pool[index]->data;
}

ConstIMethodref_t * constpool_get_imethod(ConstPool_t *pool, int index) {
	return (ConstIMethodref_t *)pool->pool[index]->data;
}

ConstFieldref_t * constpool_get_field(ConstPool_t *pool, int index) {
	return (ConstFieldref_t *)pool->pool[index]->data;
}

ConstPool_t * constpool_read(unsigned char *ptr, int *readed) {
	int start = (int)ptr;
	
	ConstPool_t 	*pool;
	unsigned char 	tag;
	void           *data;
	int 			i;
	u2  			count; 
	
	u4				tmp4;
	u4				tmp42;
	u8              tmp8;
	
	READ_U2(count, ptr);
	
	pool = constpool_create(count - 1);
	
	for (i = 1; i < count; i++) {
		READ_U1(tag, ptr);
		
		//printf("I = %i, tag = %i ", i, tag);
		
		switch (tag) {
			case CONST_UTF8:
				READ_U2(tmp4, ptr);
				// TMP4 = length
				// TMP42 = buffer
				tmp42 = (int)jvm_malloc(tmp4 + 1);
				((u1*)tmp42)[tmp4] = 0;
				memcpy((void*)tmp42, (void*)ptr, tmp4);
				ptr += tmp4;
				data = (void *)constobject_str(tmp4, (char*)tmp42);
				
				//printf("UTF8: %s\n", tmp42); 
				
				break;
			
			case CONST_INTEGER:
				READ_U4(tmp4, ptr);
				data = (void *)constobject_integer(tmp4);
				
				//printf("Integer: %i\n", tmp4);
				
				break;
				
			case CONST_FLOAT:
				READ_U4(tmp4, ptr);
				data = (void *)constobject_float(tmp4);
				
				//printf("Float: %f\n", tmp4);
				
				break;
				
			case CONST_DOUBLE:
				READ_U8(tmp8, ptr);
				data = (void *)constobject_double(tmp8);
				break;
				
			case CONST_LONG:
				READ_U8(tmp8, ptr);
				data = (void *)constobject_long(tmp8);
				break;
				
			case CONST_CLASS:
				READ_U2(tmp4, ptr);
				data = (void *)constobject_class(tmp4);
				
				//printf("Class: %i\n", tmp4);
				
				break;
				
			case CONST_STRING:
				READ_U2(tmp4, ptr);
				data = (void *)constobject_string(tmp4);
				break;
				
			case CONST_FIELDREF:
				READ_U2(tmp4, ptr);
				READ_U2(tmp42, ptr);
				
				data = (void *)constobject_field(tmp4, tmp42);
				
				//printf("Field ref: %i %i \n", tmp4, tmp42);
				
				break;
				
			case CONST_METHODREF:
				READ_U2(tmp4, ptr);
				READ_U2(tmp42, ptr);
				
				data = (void *)constobject_field(tmp4, tmp42);
				
				//printf("Method ref: %i %i \n", tmp4, tmp42);
				
				break;
				
			case CONST_IMETHODREF:
				READ_U2(tmp4, ptr);
				READ_U2(tmp42, ptr);
				
				data = (void *)constobject_field(tmp4, tmp42);
				
				//printf("IMethod: %i %i \n", tmp4, tmp42);
				
				break;						
				
			case CONST_NAMEANDTYPE:
				READ_U2(tmp4, ptr);
				READ_U2(tmp42, ptr);
				
				//printf("Name and type: %i %i \n", tmp4, tmp42);
				
				data = (void *)constobject_sign(tmp4, tmp42);
				break;				
				
			default:
				printf("Wrong tag: I = %i, %i\n",i,  tag);
				break;
		}
		
		pool->pool[i] = constobject_create(tag, data);
	}
	
	*readed = (int)(ptr - start);
	
	return pool;
}
