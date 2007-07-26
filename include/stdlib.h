/*
  include/stdlib.h
  Copyright (C) 2007 Oleg Fedorov
 */

#ifndef _STDLIB_H
#define _STDLIB_H

#include <types.h>

asmlinkage void exit(int status);

asmlinkage void *malloc(size_t size);
asmlinkage void free(void *ptr);
asmlinkage void *realloc(void *ptr, size_t size);


#endif
