/*
  include/stdarg.h
*/

#ifndef _STDARG_H
#define _STDARG_H

typedef char *va_list;

#define __va_rounded_size(TYPE)  \
  (((sizeof (TYPE) + sizeof (int) - 1) / sizeof (int)) * sizeof (int))

#define va_start(AP, LASTARG) 						\
 (AP = ((char *) &(LASTARG) + __va_rounded_size (LASTARG)))

void va_end(va_list);		/* Defined in gnulib */
#define va_end(AP)

#define va_arg(AP, TYPE)						\
 (AP += __va_rounded_size (TYPE),					\
  *((TYPE *) (AP - __va_rounded_size (TYPE))))

#include <stdio.h>
userlinkage int vsprintf(char *buf, const char *fmt, va_list args);
userlinkage int vfprintf(FILE *file, const char *fmt, va_list args);
userlinkage int sprintf(char *str, const char *fmt, ...);
userlinkage int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);
#endif				/* _STDARG_H */
