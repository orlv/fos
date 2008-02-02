#include <stdio.h>

int fputc(int c, FILE *stream) {
	char buf[1];
	buf[0] = c;
	fwrite(buf, 1, 1, stream);
	return c;
}
