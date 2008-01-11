/*
  kernel/include/stdio.h
  Copyright (C) 2006 Oleg Fedorov
*/

#ifndef _STDIO_H
#define _STDIO_H

#include <types.h>

userlinkage int printf(const char *fmt, ...);
userlinkage int sprintf(char *str, const char *fmt, ...);
userlinkage void fgets(char *buf, size_t size, int handle);
extern int stdin, stdout; 
#endif
