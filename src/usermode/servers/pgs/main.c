/*
 * Portable Graphics System
 *       GUI system
 * Copyright (c) 2007 Grindars
 */

#include <stdio.h>
#include <gui/types.h>
#include <gui/al.h>
#include <fos/fos.h>
#include <sched.h>

#include "windowing.h"
#include "cursor.h"

int need_cursor = 0;
picture_t *busy, *cursor, *close_button;
extern context_t *backbuf;
context_t screen;
int main(int argc, char *argv) {
	printf("Portable Graphics System version " VERSION " started up\n");
	printf("Loading resources..\n");
	cursor = load_file("/usr/share/cursors/cursor.pct");
	busy = load_file("/usr/share/cursors/busy.pct");
	close_button = load_file("/usr/share/pixmaps/close.pct");
	if(!cursor || !busy || !close_button) {
		printf("Error loading some graphics file\n");
		return 1;
	}
	screen = graphics_init();	
	printf("Mode set: %ux%ux%u\n",screen.w, screen.h, screen.bpp * 8);
	mousex = screen.w / 2;
	mousey = screen.h / 2;
	init_windowing();
	StartEventHandling();
	SetBusy(-1);
	exec("/usr/bin/taskbar", NULL);
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
