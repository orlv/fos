/*
  fos/stddef.h
*/

#ifndef _FOS_STDDEF_H
#define _FOS_STDDEF_H

#undef offsetof
#ifdef __compiler_offsetof
#define offsetof(TYPE,MEMBER) __compiler_offsetof(TYPE,MEMBER)
#else
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

#define typeof __typeof__

#endif
