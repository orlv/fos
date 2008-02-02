#include <stdio.h>

int fgetc(FILE *stream) {
	char buf[1];
	fread(buf, 1, 1, stream);
	return buf[0];
}

