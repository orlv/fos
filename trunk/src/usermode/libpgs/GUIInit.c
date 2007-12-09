#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
fd_t __gui;
fd_t __gui_canvas;
char *__fnt = NULL;
void GUIInit() {
	int gui_handle;
	do {
		gui_handle = open("/dev/pgs", 0);
		sched_yield();
	} while(!gui_handle);
	__gui = (fd_t )gui_handle;

	int gui_canvas_h;
	do {
		gui_canvas_h = open("/dev/pgs_canvas", 0);
		sched_yield();
	} while(!gui_handle);
	__gui_canvas = (fd_t )gui_canvas_h; 
}
