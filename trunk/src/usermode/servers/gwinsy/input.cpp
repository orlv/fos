/*
 * Copyright (C) 2008 Sergey Gridassov
 */

#include <fos/fos.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <fos/message.h>
#include "input.h"
#include "assert.h"
#include "cursor.h"
#include "windows.h"

struct mouse_pos {
  signed int dx;
  signed int dy;
  signed int dz;
  int b;
};

void mouse_thread() {
	struct mouse_pos pos;
	int hndl = -1;
	do {
		hndl = open("/dev/psaux", 0);
	} while(hndl == -1);

	while(1) {
		read(hndl, &pos, sizeof(mouse_pos));
		if(pos.dx || pos.dy) {
			cursor_shift(pos.dx, pos.dy);
			RequestRedraw(REDRAW_CURSOR, 0);
		}
	}

}


void input_init() {
	thread_create((off_t) mouse_thread, 1234);
}
