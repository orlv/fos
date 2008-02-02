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

userlinkage unsigned long int strtoul(const char *nptr, char **endptr, int base);

userlinkage char *getenv(const char *name);
userlinkage int setenv(const char *name, const char *value, int replace);
userlinkage int unsetenv (const char * name);
userlinkage int putenv(char *string);
userlinkage char * realpath (const char *name, char *resolved);
userlinkage long int atol(const char* s);
#define atoi(s) atol(s)
#endif
