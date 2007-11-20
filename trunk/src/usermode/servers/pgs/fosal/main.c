#include <fos/message.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

#include <gui/types.h>

#include "vbe.h"

int screen_width, screen_height;
context_t graphics_init() {
	printf("Configuring VBE\n");
	vbe_set_mode(0x4117);
	unsigned short *lfb;
	context_t screen;
	lfb = (unsigned short *) kmmap(0, vbe->x_resolution * vbe->y_resolution * 2, 0, vbe->phys_base_addr);
	if(!lfb) {
		printf("FOSAL: failed mapping LFB\n");
		exit(1);
	}
	screen.w= vbe->x_resolution;
	screen.h = vbe->y_resolution;
	screen.bpp = 2;
	screen.native_pixels = 0;
	screen.data = lfb;
	screen_width = vbe->x_resolution;
	screen_height = vbe->y_resolution;
	return screen;
}
