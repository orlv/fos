/*
  kernel/include/stdio.h
  Copyright (C) 2006 Oleg Fedorov
*/

#ifndef _STDIO_H
#define _STDIO_H

#include <types.h>

userlinkage int printf(const char *fmt, ...);
userlinkage int sprintf(char *str, const char *fmt, ...);
extern int stdin, stdout; 
#endif
