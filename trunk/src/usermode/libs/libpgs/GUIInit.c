#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
#include <fos/message.h>
fd_t __gui;
fd_t __gui_canvas;
char *__fnt = NULL;
#include "privatetypes.h"
int screen_width, screen_height;
void GUIInit() {
	int gui_handle;
	do {
		gui_handle = open("/dev/pgs/main", 0);
		sched_yield();
	} while(!gui_handle);
	__gui = (fd_t )gui_handle;

	int gui_canvas_h;
	do {
		gui_canvas_h = open("/dev/pgs/mapping", 0);
		sched_yield();
	} while(!gui_canvas_h);
	__gui_canvas = (fd_t )gui_canvas_h; 
	struct message msg;
	msg.flags = 0;
	msg.send_size = 0;
	msg.recv_size = 0;
	msg.tid = __gui->thread;
	msg.arg[0] = WIN_CMD_SCREEN_INFO;
	send(&msg);
	screen_width = msg.arg[0];
	screen_height = msg.arg[1];
}
