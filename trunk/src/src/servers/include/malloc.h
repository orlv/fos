/*
  Copyright (C) 2007 Oleg Fedorov
*/

#ifndef __MALLOC_H
#define __MALLOC_H

#include <types.h>

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
