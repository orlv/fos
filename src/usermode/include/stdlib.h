/*
  include/stdlib.h
  Copyright (C) 2007 Oleg Fedorov
 */

#ifndef _STDLIB_H
#define _STDLIB_H

#include <types.h>

asmlinkage void exit(int status);

#ifdef __cplusplus
extern "C"
#endif
void *malloc(size_t size);

#ifdef __cplusplus
extern "C"
#endif
void free(void *ptr);

#ifdef __cplusplus
extern "C"
#endif
void *realloc(void *ptr, size_t size);


#endif
