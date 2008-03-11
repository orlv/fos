/* vsprintf.c -- Lars Wirzenius & Linus Torvalds. */
/*
 * Wirzenius wrote this portably, Torvalds fucked it up :-)
 */

#include <stdarg.h>
#include <string.h>
#include <sched.h>
#include <stdio.h>
/* we use this so that we can do without the ctype library */
#define is_digit(c)	((c) >= '0' && (c) <= '9')

static int skip_atoi(const char **s)
{
  int i = 0;

  while (is_digit(**s))
    i = i * 10 + *((*s)++) - '0';
  return i;
}

#define ZEROPAD	1		/* pad with zero */
#define SIGN	2		/* unsigned/signed long */
#define PLUS	4		/* show plus */
#define SPACE	8		/* space if plus */
#define LEFT	16		/* left justified */
#define SPECIAL	32		/* 0x */
#define SMALL	64		/* use 'abcdef' instead of 'ABCDEF' */

#define do_div(n,base) ({ \
int __res; \
__asm__("divl %4":"=a" (n),"=d" (__res):"0" (n),"1" (0),"r" (base)); \
__res; })

int number(FILE *file, int num, int base, int size, int precision, int type)
{
  int written = 0;
  char c, sign, tmp[36];
  const char *digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  int i;

  if (type & SMALL)
    digits = "0123456789abcdefghijklmnopqrstuvwxyz";
  if (type & LEFT)
    type &= ~ZEROPAD;
  if (base < 2 || base > 36)
    return 0;
  c = (type & ZEROPAD) ? '0' : ' ';
  if (type & SIGN && num < 0) {
    sign = '-';
    num = -num;
  } else
    sign = (type & PLUS) ? '+' : ((type & SPACE) ? ' ' : 0);
  if (sign)
    size--;
  if (type & SPECIAL) {
    if (base == 16)
      size -= 2;
    else if (base == 8)
      size--;
  }
  i = 0;
  if (num == 0)
    tmp[i++] = '0';
  else
    while (num != 0)
      tmp[i++] = digits[do_div(num, base)];
  if (i > precision)
    precision = i;
  size -= precision;
  if (!(type & (ZEROPAD + LEFT))) {
    written += size;
    while (size-- > 0)
      unlocked_fputc(' ', file);
    }
  if (sign)
    unlocked_fputc(sign, file);
  if (type & SPECIAL) {
    if (base == 8) {
      written ++;
      unlocked_fputc('0', file);
    }
    else if (base == 16) {
      written += 2;
      unlocked_fputc('0', file);
      unlocked_fputc(digits[33], file);
    }
  }
  if (!(type & LEFT)) {
    written += size;
    while (size-- > 0)
      unlocked_fputc(c, file);
  }
  written += precision;
  while (i < precision--)
    unlocked_fputc('0', file);
  written += i;
  while (i-- > 0)
    unlocked_fputc(tmp[i], file);
  written += size;
  while (size-- > 0)
    unlocked_fputc(' ', file);
  return written;
}

int vfprintf(FILE *file, const char *fmt, va_list args)
{
	__fopen_fd *fd = file;
	while(!mutex_try_lock(&fd->using_mutex))
		sched_yield();
  int written = 0;
  int len;
//  int i;
  char *s;
  int *ip;

  int flags;			/* flags to number() */

  int field_width;		/* width of output field */
  int precision;		/* min. # of digits for integers; max
				   number of chars for from char **/
  int qualifier;		/* 'h', 'l', or 'L' for integer fields */

  for (; *fmt; ++fmt) {
    if (*fmt != '%') {
      written++;
      unlocked_fputc(*fmt, file);
      continue;
    }

    /* process flags */
    flags = 0;
  repeat:
    ++fmt;			/* this also skips first '%' */
    switch (*fmt) {
    case '-':
      flags |= LEFT;
      goto repeat;
    case '+':
      flags |= PLUS;
      goto repeat;
    case ' ':
      flags |= SPACE;
      goto repeat;
    case '#':
      flags |= SPECIAL;
      goto repeat;
    case '0':
      flags |= ZEROPAD;
      goto repeat;
    }

    /* get field width */
    field_width = -1;
    if (is_digit(*fmt))
      field_width = skip_atoi(&fmt);
    else if (*fmt == '*') {
      /* it's the next argument */
      field_width = va_arg(args, int);
      if (field_width < 0) {
	field_width = -field_width;
	flags |= LEFT;
      }
    }

    /* get the precision */
    precision = -1;
    if (*fmt == '.') {
      ++fmt;
      if (is_digit(*fmt))
	precision = skip_atoi(&fmt);
      else if (*fmt == '*') {
	/* it's the next argument */
	precision = va_arg(args, int);
      }
      if (precision < 0)
	precision = 0;
    }

    /* get the conversion qualifier */
    qualifier = -1;
    if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L') {
      qualifier = *fmt;
      ++fmt;
    }

    switch (*fmt) {
    case 'c':
      if (!(flags & LEFT)) {
        written += field_width;
	while (--field_width > 0)
	  unlocked_fputc(' ', file);
      }
      written++;
      unlocked_fputc((unsigned char)va_arg(args, int), file);
      written += field_width;
      while (--field_width > 0)
	unlocked_fputc(' ', file);
      break;

    case 's':
      s = va_arg(args, char *);
      len = strlen(s);
      if (precision < 0)
	precision = len;
      else if (len > precision)
	len = precision;

      if (!(flags & LEFT)) {
        written += field_width;
	while (len < field_width--)
	  unlocked_fputc(' ', file);
      }
      written += len;
      unlocked_fwrite(s, len, 1, file);
      written += field_width;
      while (len < field_width--)
	unlocked_fputc(' ', file);
      break;

    case 'o':
      written += number(file, va_arg(args, unsigned long), 8,
		   field_width, precision, flags);
      break;

    case 'p':
      if (field_width == -1) {
	field_width = 8;
	flags |= ZEROPAD;
      }
      written += number(file,
		   (unsigned long)va_arg(args, void *), 16,
		   field_width, precision, flags);
      break;

    case 'x':
      flags |= SMALL;
    case 'X':
      written += number(file, va_arg(args, unsigned long), 16,
		   field_width, precision, flags);
      break;

    case 'd':
    case 'i':
      flags |= SIGN;
    case 'u':
      written += number(file, va_arg(args, unsigned long), 10,
		   field_width, precision, flags);
      break;

    case 'n':
      ip = va_arg(args, int *);
      *ip = written;
      break;

    default:
      if (*fmt != '%') {
	unlocked_fputc('%', file);
	written ++;
      }
      if (*fmt) {
	unlocked_fputc(*fmt, file);
        written ++;
      } else
	--fmt;
      break;
    }
  }
  mutex_unlock(&fd->using_mutex);
  return written;
}
