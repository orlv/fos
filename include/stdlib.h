/*
  include/stdlib.h
  Copyright (C) 2007 Oleg Fedorov
 */

#ifndef _STDLIB_H
#define _STDLIB_H

#include <types.h>

userlinkage void exit(int status);

userlinkage void *malloc(size_t size);
userlinkage void free(void *ptr);
userlinkage void *realloc(void *ptr, size_t size);

#define RAND_MAX 	0x7ffffffe
userlinkage long int random(void);
userlinkage void srandom(unsigned int seed);

#endif
