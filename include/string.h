/*
  include/string.h
  Основано на string.h из linux-2.6.17
*/

#ifndef __STRING_H
#define __STRING_H

#include <types.h>

static inline char *strcpy(char *dest, const char *src)
{
  int d0, d1, d2;
  __asm__ __volatile__("1:\tlodsb\n\t"
		       "stosb\n\t"
		       "testb %%al,%%al\n\t"
		       "jne 1b":"=&S"(d0), "=&D"(d1), "=&a"(d2)
		       :"0"(src), "1"(dest):"memory");
  return dest;
}

static inline char *strncpy(char *dest, const char *src, size_t count)
{
  int d0, d1, d2, d3;
  __asm__ __volatile__("1:\tdecl %2\n\t"
		       "js 2f\n\t"
		       "lodsb\n\t"
		       "stosb\n\t"
		       "testb %%al,%%al\n\t"
		       "jne 1b\n\t"
		       "rep\n\t"
		       "stosb\n" "2:":"=&S"(d0), "=&D"(d1), "=&c"(d2), "=&a"(d3)
		       :"0"(src), "1"(dest), "2"(count):"memory");
  return dest;
}

static inline char *strcat(char *dest, const char *src)
{
  int d0, d1, d2, d3;
  __asm__ __volatile__("repne\n\t"
		       "scasb\n\t"
		       "decl %1\n"
		       "1:\tlodsb\n\t"
		       "stosb\n\t"
		       "testb %%al,%%al\n\t"
		       "jne 1b":"=&S"(d0), "=&D"(d1), "=&a"(d2), "=&c"(d3)
		       :"0"(src), "1"(dest), "2"(0), "3"(0xffffffffu):"memory");
  return dest;
}

static inline char *strncat(char *dest, const char *src, size_t count)
{
  int d0, d1, d2, d3;
  __asm__ __volatile__("repne\n\t"
		       "scasb\n\t"
		       "decl %1\n\t"
		       "movl %8,%3\n"
		       "1:\tdecl %3\n\t"
		       "js 2f\n\t"
		       "lodsb\n\t"
		       "stosb\n\t"
		       "testb %%al,%%al\n\t"
		       "jne 1b\n"
		       "2:\txorl %2,%2\n\t"
		       "stosb":"=&S"(d0), "=&D"(d1), "=&a"(d2), "=&c"(d3)
		       :"0"(src), "1"(dest), "2"(0), "3"(0xffffffffu),
		       "g"(count)
		       :"memory");
  return dest;
}

static inline int strcmp(const char *cs, const char *ct)
{
  int d0, d1;
  register int __res;
  __asm__ __volatile__("1:\tlodsb\n\t"
		       "scasb\n\t"
		       "jne 2f\n\t"
		       "testb %%al,%%al\n\t"
		       "jne 1b\n\t"
		       "xorl %%eax,%%eax\n\t"
		       "jmp 3f\n"
		       "2:\tsbbl %%eax,%%eax\n\t"
		       "orb $2,%%al\n" "3:":"=a"(__res), "=&S"(d0), "=&D"(d1)
		       :"1"(cs), "2"(ct)
		       :"memory");
  return __res;
}

static inline int strncmp(const char *cs, const char *ct, size_t count)
{
  register int __res;
  int d0, d1, d2;
  __asm__ __volatile__("1:\tdecl %3\n\t"
		       "js 2f\n\t"
		       "lodsb\n\t"
		       "scasb\n\t"
		       "jne 3f\n\t"
		       "testb %%al,%%al\n\t"
		       "jne 1b\n"
		       "2:\txorl %%eax,%%eax\n\t"
		       "jmp 4f\n"
		       "3:\tsbbl %%eax,%%eax\n\t"
		       "orb $2,%%al\n"
		       "4:":"=a"(__res), "=&S"(d0), "=&D"(d1), "=&c"(d2)
		       :"1"(cs), "2"(ct), "3"(count)
		       :"memory");
  return __res;
}

static inline char * strchr(const char * s, int c)
{
int d0;
register char * __res;
__asm__ __volatile__(
	"movb %%al,%%ah\n"
	"1:\tlodsb\n\t"
	"cmpb %%ah,%%al\n\t"
	"je 2f\n\t"
	"testb %%al,%%al\n\t"
	"jne 1b\n\t"
	"movl $1,%1\n"
	"2:\tmovl %1,%0\n\t"
	"decl %0"
	:"=a" (__res), "=&S" (d0)
	:"1" (s),"0" (c)
	:"memory");
return __res;
}

static inline char *strrchr(const char *s, int c)
{
  int d0, d1;
  register char *__res;
  __asm__ __volatile__("movb %%al,%%ah\n"
		       "1:\tlodsb\n\t"
		       "cmpb %%ah,%%al\n\t"
		       "jne 2f\n\t"
		       "leal -1(%%esi),%0\n"
		       "2:\ttestb %%al,%%al\n\t"
		       "jne 1b":"=g"(__res), "=&S"(d0), "=&a"(d1)
		       :"0"(0), "1"(s), "2"(c)
		       :"memory");
  return __res;
}

static inline size_t strlen(const char *s)
{
  int d0;
  register int __res;
  __asm__ __volatile__("repne\n\t"
		       "scasb\n\t"
		       "notl %0\n\t" "decl %0":"=c"(__res), "=&D"(d0)
		       :"1"(s), "a"(0), "0"(0xffffffffu)
		       :"memory");
  return __res;
}

static inline size_t strnlen(const char * s, size_t count)
{
  int d0;
  register int __res;
  __asm__ __volatile__(
		       "movl %2,%0\n\t"
		       "jmp 2f\n"
		       "1:\tcmpb $0,(%0)\n\t"
		       "je 3f\n\t"
		       "incl %0\n"
		       "2:\tdecl %1\n\t"
		       "cmpl $-1,%1\n\t"
		       "jne 1b\n"
		       "3:\tsubl %2,%0"
		       :"=a" (__res), "=&d" (d0)
		       :"c" (s),"1" (count)
		       :"memory");
  return __res;
}

static inline void *memcpy(void *to, const void *from, size_t n)
{
  int d0, d1, d2;
  __asm__ __volatile__("rep ; movsl\n\t" "movl %4,%%ecx\n\t" "andl $3,%%ecx\n\t"
#if 1				/* want to pay 2 byte penalty for a chance to skip microcoded rep? */
		       "jz 1f\n\t"
#endif
		       "rep ; movsb\n\t" "1:":"=&c"(d0), "=&D"(d1), "=&S"(d2)
		       :"0"(n / 4), "g"(n), "1"((long)to), "2"((long)from)
		       :"memory");
  return (to);
}

static inline void * memset(void * s, char c, size_t count)
{
  int d0, d1;
  __asm__ __volatile__(
		       "rep\n\t"
		       "stosb"
		       : "=&c" (d0), "=&D" (d1)
		       :"a" (c),"1" (s),"0" (count)
		       :"memory");
  return s;
}

static inline int memcmp(const void * cs,const void * ct,int count)
{
register int __res;
__asm__("cld\n\t"
	"repe\n\t"
	"cmpsb\n\t"
	"je 1f\n\t"
	"movl $2,%%eax\n\t"
	"jl 1f\n\t"
	"negl %%eax\n"
	"1:"
	:"=a" (__res):"0" (0),"D" (cs),"S" (ct),"c" (count));
return __res;
}

static inline void * memchr(const void * cs,int c,size_t count)
{
int d0;
register void * __res;
if (!count)
	return NULL;
__asm__ __volatile__(
	"repne\n\t"
	"scasb\n\t"
	"je 1f\n\t"
	"movl $1,%0\n"
	"1:\tdecl %0"
	:"=D" (__res), "=&c" (d0)
	:"a" (c),"0" (cs),"1" (count)
	:"memory");
return __res;
}

#ifdef __cplusplus
extern "C" {
#endif
  char*strtok_r(char*s,const char*delim,char**ptrptr);
  size_t strspn(const char *s, const char *accept);
  size_t strcspn(const char *s, const char *reject);
  char *strsep(char **stringp, const char *delim);
  void *memmove(void *dst, const void *src, size_t count);
  char *strdup(const char *s);
  char *strstr(const char *haystack, const char *needle);
#ifdef __cplusplus
}
#endif

#endif
