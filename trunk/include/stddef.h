#ifndef _STDDEF_H
#define _STDDEF_H

#undef NULL
#if defined(__cplusplus)
#define NULL 0
#else
#define NULL ((void *)0)
#endif

typedef int ptrdiff_t;

#endif