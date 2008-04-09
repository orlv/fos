/*
 * Copyright (C) 2008 Sergey Gridassov
 */

#include <fos/nsi.h>
#include <fos/fos.h>
#include <fos/fs.h>
#include <fos/message.h>
#include <sched.h>
#include <stdio.h>
#include "context.h"
#include "video.h"
#include "assert.h"
#include "cursor.h"
#include "windows.h"
#include "picture.h"
#include "util.h"
#include "ipc.h"

#define SEND_REQUEST	(BASE_METHOD_N + 0)
#define TRIGGER_MMOVE	0
#define TRIGGER_UP	1
#define TRIGGER_DOWN	2

typedef struct {
	picture_t *pict;
	const char *filename;
	int hot_x;
	int hot_y;
} cursor_t;

static cursor_t cursor_table[] = {
/* CURSOR_POINTER */ { NULL, "/usr/share/cursors/pointer.pct", 0, 0 },
};

static cursor_t *current = NULL;

static int old_x, old_y, cur_x, cur_y;

static volatile tid_t sending_thread = 0;

static void RequestSend(int type, int x, int y) {
	struct message msg;
	msg.flags = 0;
	msg.send_size = 0;
	msg.recv_size = 0;
	msg.arg[0] = SEND_REQUEST;
	msg.arg[1] = type;
	msg.arg[2] = x;
	msg.arg[3] = y;
	msg.tid = sending_thread;
	send(&msg);
}


int cursor_select(unsigned int type) {
	if(type >= sizeof(cursor_table) / sizeof(cursor_t))
		return -1;

	current = &cursor_table[type];
	return 0;
}

void cursor_move(int x, int y) {
	old_x = cur_x;
	old_y = cur_y;
	cur_x = x;
	cur_y = y;

	int dx = cur_x - old_x;	// смысл в том, чтобы в событии
	int dy = cur_y - old_y;	// d[xy] не выходили за экран

	PostEvent(0, 0, EV_GLOBAL, EVG_MMOVE, cur_x, cur_y, dx, dy);
	RequestSend(TRIGGER_MMOVE, cur_x, cur_y);
}

void cursor_sync() {
	Blit(backbuf, screen, old_x - current->hot_x, old_y - current->hot_y, current->pict->width, current->pict->height, old_x - current->hot_x, old_y - current->hot_y);
	draw_picture(current->pict, cur_x - current->hot_x, cur_y - current->hot_y, screen);
}

static int ProcessSendRequest(struct message *msg) {
	msg->send_size = 0;
	reply(msg);
	switch(msg->arg[1]) {
		case TRIGGER_MMOVE:
			windows_handle_move(msg->arg[2], msg->arg[3]);
			break;
		case TRIGGER_UP:
			// TODO
			break;
		case TRIGGER_DOWN:
			// TODO
			break;
	}
	return 0;
}


static void SendingThread() {
	nsi_t *interface = new nsi_t();

	interface->add(SEND_REQUEST, ProcessSendRequest);
	
	sending_thread = my_tid();

	while (1) 
		interface->wait_message();
}

void cursor_init(void) {
	thread_create((off_t) & SendingThread, 0);
	while(!sending_thread) 
		sched_yield();
	
	for(unsigned int i = 0; i < sizeof(cursor_table) / sizeof(cursor_t); i++) {
		cursor_table[i].pict = (picture_t *) load_file(cursor_table[i].filename);
		assert(cursor_table[i].pict != 0);
		
		cursor_table[i].hot_x = cursor_table[i].pict->hot_x;
		cursor_table[i].hot_y = cursor_table[i].pict->hot_y;
	}
	cursor_move(screen->w / 2, screen->h / 2);
	cursor_select(CURSOR_POINTER);
	printf("cursor started\n");
}

void cursor_shift(int dx, int dy) {
	old_x = cur_x;
	old_y = cur_y;
	cur_x+= dx;
	cur_y+= dy;
	
	if(cur_x < 0)
		cur_x = 0;

	if(cur_y < 0)
		cur_y = 0;

	if(cur_x > screen->w)
		cur_x = screen->w;

	if(cur_y > screen->h)
		cur_y = screen->h;

	dx = cur_x - old_x;	// смысл в том, чтобы в событии
	dy = cur_y - old_y;	// d[xy] не выходили за экран

	PostEvent(0, 0, EV_GLOBAL, EVG_MMOVE, cur_x, cur_y, dx, dy);
	RequestSend(TRIGGER_MMOVE, cur_x, cur_y);
}
