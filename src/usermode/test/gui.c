#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fos/message.h>
#include <fos/fs.h>

#include <string.h>
#include "gui.h"
static fd_t gui;
static fd_t gui_canvas;
void GUIInit() {
	int gui_handle;
	do {
		gui_handle = open("/dev/pgs", 0);
		sched_yield();
	} while(!gui_handle);
	gui = (fd_t )gui_handle;

	int gui_canvas_h;
	do {
		gui_canvas_h = open("/dev/pgs_canvas", 0);
		sched_yield();
	} while(!gui_handle);
	gui_canvas = (fd_t )gui_canvas_h; 
}
typedef struct {
	int parent;
	int x;
	int y;
	int w;
	int h;
	int class;
} create_win_t;
#define MAX_TITLE_LEN 64

int CreateWindow(int parent, int x, int y, int w, int h, char *caption, int flags) {
	char *buf = malloc(sizeof(create_win_t) + MAX_TITLE_LEN);
	create_win_t *struc = (create_win_t *) buf;
	char *title = buf + sizeof(create_win_t);
	strcpy(title, caption);
	struc->parent = parent;
	struc->x = x;
	struc->y = y;
	struc->w = w;
	struc->h = h;
	struc->class = flags;
	
	struct message msg;
	msg.recv_size = 0;
	msg.tid = gui->thread;
	msg.flags = 0;
	msg.arg[0] = WIN_CMD_CREATEWINDOW;
	msg.send_size = sizeof(create_win_t) + MAX_TITLE_LEN;
	msg.send_buf = buf;
	send(&msg);
	int bpp = msg.arg[1];
	int hndl = msg.arg[0];
	printf("Videobuffer: %ux%ux%u (%u bytes)\n", w, h, bpp * 8, w * h * bpp);

	char *canvas = kmmap(0, w * h * bpp, 0, 0);
	memset(canvas, 0x55, w * h * bpp);
	printf("buf: 0x%x, size: %d\n", canvas, w * h * bpp);
	msg.tid = gui_canvas->thread;
	msg.recv_size =  w * h * bpp;
	msg.recv_buf = canvas;
	msg.send_size = w * h * bpp;
	msg.send_buf = canvas;
	msg.flags = MSG_MEM_SHARE;
	msg.arg[0] = WIN_CMD_MAPBUF;
	msg.arg[1] = hndl;
	send(&msg);
	int i;
	for(;;) {
		for(i = 0; i <  w * h * bpp;i++) canvas[i]++;
		printf(".");
	}

	return hndl;
}
typedef struct {
	void * reserved[2];
	int class;
	int handle;
	int a0;
	int a1;
	int a2;
	int a3;
} event_t;
void WaitEvent(int *class, int *handle, int *a0, int *a1, int *a2, int *a3) {
	event_t event;
	struct message msg;
	msg.flags = 0;
	msg.arg[0] = WIN_CMD_WAIT_EVENT;
	msg.tid = gui->thread;
	msg.send_size = 0;
	msg.recv_size = sizeof(event_t);
	msg.recv_buf = &event;
	send(&msg);
	*class = event.class;
	*handle = event.handle;
	*a0 = event.a0;
	*a1 = event.a1;
	*a2 = event.a2;
	*a3 = event.a3;


}
void DestroyWindow(int handle) {
	struct message msg;
	msg.flags = 0;
	msg.send_size = 0;
	msg.recv_size = 0;
	msg.tid = gui->thread;
	msg.arg[0] = WIN_CMD_DESTROYWINDOW;
	msg.arg[1] = handle;
	send(&msg);
}
void GuiEnd() {
	struct message msg;
	msg.flags = 0;
	msg.send_size = 0;
	msg.recv_size = 0;
	msg.tid = gui->thread;
	msg.arg[0] = WIN_CMD_CLEANUP;
	send(&msg);
	gui = NULL;
	gui_canvas = NULL;
}
