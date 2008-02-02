/*
  apps/include/vsprintf.h
  Copyright (C) 2006 Oleg Fedorov
*/

#ifndef _VSPRINTF_H
#define _VSPRINTF_H

#include <types.h>
#include <stdarg.h>
#include <stdio.h>

userlinkage int vsprintf(char *buf, const char *fmt, va_list args);
userlinkage int vfprintf(FILE *file, const char *fmt, va_list args);
userlinkage int sprintf(char *str, const char *fmt, ...);

#endif
