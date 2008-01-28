/*
 * Copyright (c) 2008 Sergey Gridassov
 */

#include <stdio.h>

void rewind(FILE *stream) {
	fseek(stream, 0, SEEK_SET);
}
