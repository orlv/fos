#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pgs/pgs.h>
extern char *__fnt;
void pstring(int handle, int x, int y, int color, char *str) {
	if(!__fnt) {
		__fnt = malloc(4096);
		int h = open("/usr/share/fonts/font.psf", 0);
		lseek(h, 4, SEEK_SET);
		read(h, __fnt, 4096);
		close(h);
		
	}
	for(;*str; str++) {
		for(int i = 0; i < 16; i++)
		for(int j = 0; j < 8; j++) 
			if(__fnt[16 * (unsigned char)*str + i] & (1<<j))
				pixel(handle, x + 8 - j, y + i, color);
		x += 8;
	}
}
