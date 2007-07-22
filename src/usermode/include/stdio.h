/*
  kernel/include/stdio.h
  Copyright (C) 2006 Oleg Fedorov
*/

#ifndef _STDIO_H
#define _STDIO_H

#include <types.h>

#ifdef __cplusplus
extern "C"
#endif
int printf(const char *fmt, ...);

#ifdef __cplusplus
extern "C"
#endif
int sprintf(char *str, const char *fmt, ...);

#endif
