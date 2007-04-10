/*
  apps/include/string.h
  Взято из linux-2.6.17
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
		       "orb $1,%%al\n" "3:":"=a"(__res), "=&S"(d0), "=&D"(d1)
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
		       "orb $1,%%al\n"
		       "4:":"=a"(__res), "=&S"(d0), "=&D"(d1), "=&c"(d2)
		       :"1"(cs), "2"(ct), "3"(count)
		       :"memory");
  return __res;
}

static inline char *strchr(const char *s, int c)
{
  int d0;
  register char *__res;
  __asm__ __volatile__("movb %%al,%%ah\n"
		       "1:\tlodsb\n\t"
		       "cmpb %%ah,%%al\n\t"
		       "je 2f\n\t"
		       "testb %%al,%%al\n\t"
		       "jne 1b\n\t"
		       "movl $1,%1\n"
		       "2:\tmovl %1,%0\n\t" "decl %0":"=a"(__res), "=&S"(d0)
		       :"1"(s), "0"(c)
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

#if 0
static __always_inline void *__memcpy(void *to, const void *from, size_t n)
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
#endif

/*
 * This looks ugly, but the compiler can optimize it totally,
 * as the count is constant.
 */
static inline void *memcpy(void *to, const void *from, size_t n)
{
  long esi, edi;
  if (!n)
    return to;
#if 1				/* want to do small copies with non-string ops? */
  switch (n) {
  case 1:
    *(char *)to = *(char *)from;
    return to;
  case 2:
    *(short *)to = *(short *)from;
    return to;
  case 4:
    *(int *)to = *(int *)from;
    return to;
#if 1				/* including those doable with two moves? */
  case 3:
    *(short *)to = *(short *)from;
    *((char *)to + 2) = *((char *)from + 2);
    return to;
  case 5:
    *(int *)to = *(int *)from;
    *((char *)to + 4) = *((char *)from + 4);
    return to;
  case 6:
    *(int *)to = *(int *)from;
    *((short *)to + 2) = *((short *)from + 2);
    return to;
  case 8:
    *(int *)to = *(int *)from;
    *((int *)to + 1) = *((int *)from + 1);
    return to;
#endif
  }
#endif
  esi = (long)from;
  edi = (long)to;
  if (n >= 5 * 4) {
    /* large block: use rep prefix */
    int ecx;
    __asm__ __volatile__("rep ; movsl":"=&c"(ecx), "=&D"(edi), "=&S"(esi)
			 :"0"(n / 4), "1"(edi), "2"(esi)
			 :"memory");
  } else {
    /* small block: don't clobber ecx + smaller code */
    if (n >= 4 * 4)
      __asm__ __volatile__("movsl":"=&D"(edi), "=&S"(esi):"0"(edi),
			   "1"(esi):"memory");
    if (n >= 3 * 4)
      __asm__ __volatile__("movsl":"=&D"(edi), "=&S"(esi):"0"(edi),
			   "1"(esi):"memory");
    if (n >= 2 * 4)
      __asm__ __volatile__("movsl":"=&D"(edi), "=&S"(esi):"0"(edi),
			   "1"(esi):"memory");
    if (n >= 1 * 4)
      __asm__ __volatile__("movsl":"=&D"(edi), "=&S"(esi):"0"(edi),
			   "1"(esi):"memory");
  }
  switch (n % 4) {
    /* tail */
  case 0:
    return to;
  case 1:
  __asm__ __volatile__("movsb": "=&D"(edi), "=&S"(esi): "0"(edi), "1"(esi):"memory");
    return to;
  case 2:
  __asm__ __volatile__("movsw": "=&D"(edi), "=&S"(esi): "0"(edi), "1"(esi):"memory");
    return to;
  default:
  __asm__ __volatile__("movsw\n\tmovsb": "=&D"(edi), "=&S"(esi): "0"(edi), "1"(esi):"memory");
    return to;
  }
}

#endif
