/*
 * Copyright (C) 2008 Sergey Gridassov
 */

#include <stdio.h>
#include <stdlib.h>

void __assert_fail (const char *expr, const char *file, unsigned int line, const char *function) {
	fprintf(stderr, "assertion failed in file %s, function %s, line %u\n", file, line, function ? function : "[unknown]");
	fflush(stderr);
	// abort(); по стандарту, но не реализовано
	exit(1);
}
