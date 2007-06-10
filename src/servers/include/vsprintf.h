/*
  apps/include/vsprintf.h
  Copyright (C) 2006 Oleg Fedorov
*/

#ifndef _VSPRINTF_H
#define _VSPRINTF_H

#include <stdarg.h>

#ifdef __cplusplus
extern "C"
#endif
int vsprintf(char *buf, const char *fmt, va_list args);

#ifdef __cplusplus
extern "C"
#endif
int sprintf(char *str, const char *fmt, ...);

#endif
