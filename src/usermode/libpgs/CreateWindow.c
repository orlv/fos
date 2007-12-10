#include <fos/message.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include "privatetypes.h"

extern fd_t __gui;
extern fd_t __gui_canvas;
int CreateWindow(int w, int h, char *caption, int flags, int *evhndl) {
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
	msg.tid = __gui->thread;
	msg.flags = 0;
	msg.arg[0] = WIN_CMD_CREATEWINDOW;
	msg.send_size = sizeof(create_win_t) + MAX_TITLE_LEN;
	msg.send_buf = buf;
	send(&msg);
	int hndl = wi.handle;
	*evhndl = hndl;
	int size = ALIGN((w + wi.margin_left + wi.margin_right) * (h + wi.margin_up + wi.margin_down) * wi.bpp, 4096);
	char *canvas = kmmap(0, size, 0, 0);
	msg.tid = __gui_canvas->thread;
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
