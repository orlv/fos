#include <stdio.h>

int unlocked_fputc(int c, FILE *stream) {
	char buf[1];
	buf[0] = c;
	unlocked_fwrite(buf, 1, 1, stream);
	return c;
}
