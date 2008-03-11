#ifndef DPRINTF_H
#define DPRINTF_H
#include <sys/io.h>

static inline void dputc(const char ch) {
	while (!(inb(0x3FD) & 0x20));
	outb(ch, 0x3F8);
}

static void dputnum (unsigned long n, unsigned long base, int minlen)
{
	if (minlen > 1 || n >= base)
		dputnum(n / base, base, minlen - 1);

	dputc("0123456789ABCDEF"[n % base]);
}

static inline void dputs(const char *str) {
	for(; *str; str++)
		dputc(*str);
}

static void dprintf(const char *format, ...) {
	const char **args = &format;
	for(args++; *format; format++) {
		if(*format != '%') {
			dputc(*format);
			continue;
		}
		switch(*++format) {
		case '%':
			dputc('%');
			break;
		case 's':
			dputs(*args++);
			break;
		case 'u':
			dputnum((unsigned long)*args++, 10, 1);
			break;
		case 'x':
			dputnum((unsigned long)*args++, 16, 8);
			break;
		default:
			break;
		}
	}
}
#endif

