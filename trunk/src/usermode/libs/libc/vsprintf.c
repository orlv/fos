/*
 * Copyright (c) 2008 Sergey Gridassov
 *
 * Не используйте эту функцию! Используйте snprintf.
 */

#include <vsprintf.h>
int vsprintf(char *str, const char *format, va_list ap) {
	int size = 0xFFFFFFFF - (int) str;
	return vsnprintf(str, size, format, ap);
}
