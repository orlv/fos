/*
  apps/include/vsprintf.h
  Copyright (C) 2006 Oleg Fedorov
*/

#ifndef _VSPRINTF_H
#define _VSPRINTF_H

#include <stdarg.h>
#include <types.h>

asmlinkage int vsprintf(char *buf, const char *fmt, va_list args);
asmlinkage int sprintf(char *str, const char *fmt, ...);

#endif
