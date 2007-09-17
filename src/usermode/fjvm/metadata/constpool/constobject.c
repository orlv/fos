#include "constobject.h"
#include "../../alloc.h"

ConstObject_t *constobject_create(unsigned char tag, void *data) {
	ConstObject_t *obj = (ConstObject_t *)jvm_malloc(
		sizeof(ConstObject_t));
	
	obj->tag = tag;
	obj->data = data;
	
	return obj;
}

ConstString_t   *constobject_string(int i) {
	ConstString_t *value = (ConstString_t *)jvm_malloc(sizeof(ConstString_t));
	value->stringID = i;
	
	return value;
}

ConstInteger_t * constobject_integer(int i) {
	ConstInteger_t *value = (ConstInteger_t *)jvm_malloc(sizeof(ConstInteger_t));
	value->value = i;
	
	return value;
}

ConstSingle_t * constobject_float(int i) {
	ConstSingle_t *value = (ConstSingle_t *)jvm_malloc(sizeof(ConstSingle_t));
	value->value = i ;
	
	return value;
}

ConstDouble_t *constobject_double(long long i) {
	ConstDouble_t *value = (ConstDouble_t *)jvm_malloc(sizeof(ConstDouble_t));
	value->value = i;
	
	return value;
}

ConstLong_t *constobject_long(long long i) {
	ConstLong_t *value = (ConstLong_t *)jvm_malloc(sizeof(ConstLong_t));
	value->value = i;
	
	return value;
}

ConstClass_t    *constobject_class(int i) {
	ConstClass_t *value = (ConstClass_t *)jvm_malloc(sizeof(ConstClass_t));
	value->nameID = i;
	
	return value;
}

ConstFieldref_t *constobject_field(int i, int j) {
	ConstFieldref_t *value = (ConstFieldref_t *)jvm_malloc(sizeof(ConstFieldref_t));
	value->classID = i;
	value->signID = j;
	
	return value;
}

ConstMethodref_t*constobject_method(int i, int j) {
	ConstMethodref_t *value = (ConstMethodref_t *)jvm_malloc(sizeof(ConstMethodref_t));
	value->classID = i;
	value->signID = j;
	
	return value;
}

ConstIMethodref_t*constobject_imethod(int i, int j) {
	ConstIMethodref_t *value = (ConstIMethodref_t *)jvm_malloc(sizeof(ConstIMethodref_t));
	value->classID = i;
	value->signID = j;
	
	return value;
}

ConstSign_t * constobject_sign(int i, int j) {
	ConstSign_t *value = (ConstSign_t *)jvm_malloc(sizeof(ConstSign_t));
	value->nameID = i;
	value->descID = j;
	
	return value;
}

ConstUTF8_t * constobject_str(int i, char *d) {
	ConstUTF8_t *value = (ConstUTF8_t *)jvm_malloc(sizeof(ConstUTF8_t));
	value->data = d;
	value->length = i;
	
	return value;
}
