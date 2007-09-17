#ifndef NATIVE_H_
#define NATIVE_H_

#include "hash.h"
#include "metadata/class.h"

#define NUM_NATIVES 64

typedef struct Native_s {
	hash_t 		hclass;
	hash_t		hmethod;
	
	native_func_t native;
} Native_t;

#endif /*NATIVE_H_*/
