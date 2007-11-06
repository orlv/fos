/*
 * Portable Graphics System
 *       GUI system
 * Copyright (c) 2007 Grindars
 */

#include <stdio.h>
#include <gui/types.h>
#include <gui/al.h>

#include "windowing.h"
#include "cursor.h"

mode_definition_t mode;
int need_cursor = 0;
extern context_t *backbuf;
int main(int argc, char *argv) {
	printf("Portable Graphics System version " VERSION " started up\n");
	mode = graphics_init();	
	printf("Mode set: %ux%ux%u\n", mode.width, mode.height, mode.bpp * 8);

	init_windowing();
	StartEventHandling();
	printf("I'am alive!\n");
	exec("/mnt/modules/test", "1");
	exec("/mnt/modules/test", "2");
	exec("/mnt/modules/test", "3");
	exec("/mnt/modules/test", "4");
	for(;;) {
		if (need_refresh)
		{
		    Redraw();
		    need_refresh = 0;
		}
		if(need_cursor) {
			RedrawCursor();
			need_cursor = 0;
		}
	
	}
	return 0;
}
