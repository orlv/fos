#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fos/message.h>
#include <fos/fs.h>
#include <sched.h>
#include <string.h>
#include "gui.h"
#define ALIGN(a, b)  ((a + (b - 1)) & ~(b - 1))

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
	int w;
	int h;
	int class;
} create_win_t;

struct win_hndl{
	int handle;
	char *data;
	int w;
	int h;
	int bpp;
	int margin_up;
	int margin_down;
	int margin_left;
	int margin_right;
} ;

struct win_info {
	int bpp;
	int handle;
	int margin_up;
	int margin_down;
	int margin_left;
	int margin_right;
};
#define MAX_TITLE_LEN 64

int CreateWindow(int w, int h, char *caption, int flags) {
	char *buf = malloc(sizeof(create_win_t) + MAX_TITLE_LEN);
	create_win_t *struc = (create_win_t *) buf;
	char *title = buf + sizeof(create_win_t);
	strcpy(title, caption);
	struc->w = w;
	struc->h = h;
	struc->class = flags;
	

	struct win_info wi;

	struct message msg;
	msg.recv_size = sizeof(wi);
	msg.recv_buf = &wi;
	msg.tid = gui->thread;
	msg.flags = 0;
	msg.arg[0] = WIN_CMD_CREATEWINDOW;
	msg.send_size = sizeof(create_win_t) + MAX_TITLE_LEN;
	msg.send_buf = buf;
	send(&msg);
	int hndl = wi.handle;
	printf("Videobuffer: %ux%ux%u (%u bytes)\n", w, h, wi.bpp, w * h * wi.bpp);

	int size = ALIGN(w * h * wi.bpp, 4096);
	char *canvas = kmmap(0, size, 0, 0);
	msg.tid = gui_canvas->thread;
	msg.recv_size =  size;
	msg.recv_buf = canvas;
	msg.send_size = size;
	msg.send_buf = canvas;
	msg.flags = MSG_MEM_SHARE;
	msg.arg[0] = WIN_CMD_MAPBUF;
	msg.arg[1] = hndl;

	send(&msg);

	struct win_hndl *wh = malloc(sizeof(struct win_hndl));
	wh->margin_up = wi.margin_up;
	wh->margin_down = wi.margin_down;
	wh->margin_left = wi.margin_left;
	wh->margin_right = wi.margin_right;
	wh->handle = hndl;
	wh->data = canvas;
	wh->w = w;
	wh->h = h;
	wh->bpp = wi.bpp;
	return (int) wh;
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
	struct win_hndl *wh  = (struct win_hndl *) handle;
	struct message msg;
	msg.flags = 0;
	msg.send_size = 0;
	msg.recv_size = 0;
	msg.tid = gui->thread;
	msg.arg[0] = WIN_CMD_DESTROYWINDOW;
	msg.arg[1] = wh->handle;
	send(&msg);
	free(wh);
	kmunmap((off_t) wh->data, ALIGN(wh->w * wh->h * wh->bpp, 4096));
}
void RefreshWindow(int handle) {
	struct win_hndl *wh  = (struct win_hndl *) handle;
	struct message msg;
	msg.flags = 0;
	msg.send_size = 0;
	msg.recv_size = 0;
	msg.tid = gui->thread;
	msg.arg[0] = WIN_CMD_REFRESHWINDOW;
	msg.arg[1] = wh->handle;
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
#define RED(x, bits)	((x >> (16 + 8 - bits)) & ((1 << bits) - 1))
#define GREEN(x, bits)	((x >> (8 + 8 - bits)) & ((1 << bits) - 1))
#define BLUE(x, bits)	((x >> (8 - bits)) & ((1 << bits) - 1))
void pixel(int handle, int x, int y, int color) {
	struct win_hndl *wh  = (struct win_hndl *) handle;
	x += wh->margin_left;
	y += wh->margin_up;
	if(x > wh->w - wh->margin_right || y > wh->h - wh->margin_down || y < 0 || x < 0) return;
	unsigned short *ptr = (unsigned short *) wh->data;
	ptr[x + y * wh->w] =  RED(color, 5) << 11 | GREEN(color, 6) << 5 | BLUE(color, 5);
}
