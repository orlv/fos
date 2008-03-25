/*
 * Copyright (C) 2008 Sergey Gridassov
 */

#include <stdlib.h>
#include <string.h>
#include <fos/fos.h>
#include <fos/fs.h>
#include <fos/message.h>
#include <sys/mman.h>
#include <fos/nsi.h>
#include <stdio.h>
#include "windows.h"
#include "context.h"
#include "video.h"
#include "assert.h"
#include "cursor.h"

#define REDRAW_REQUEST	(BASE_METHOD_N + 0)

context_t *backbuf, *locate;
tid_t redraw_thread;

int windows_init() {
	size_t screen_bytes = screen->w * screen->h * screen->bpp;
	backbuf = new context_t;
	assert(backbuf != NULL);


	backbuf->w = screen->w;
	backbuf->h = screen->h;
	backbuf->bpp = screen->bpp;
	backbuf->native_pixels = 0;
	backbuf->data = kmmap(0, screen_bytes, 0, 0);

	assert(backbuf->data != NULL);

	locate = new context_t;
	assert(locate != NULL);


	locate->w = screen->w;
	locate->h = screen->h;
	locate->bpp = screen->bpp;
	backbuf->native_pixels = 1;
	locate->data = kmmap(0, screen_bytes, 0, 0);

	assert(locate->data != NULL);

	redraw_thread = my_tid();

	RequestRedraw(REDRAW_FULL, 0);

	return 0;
}

static void FullRedraw() {
	Rect(0, 0, backbuf->w, backbuf->h, 0x4d6aff, backbuf);


	Blit(backbuf, screen, 0, 0, backbuf->w, backbuf->h, 0, 0);
}

static void PartialRedraw(int hndl) {

}

static int ProcessRedrawRequest(struct message *msg) {
	msg->arg[2] = NO_ERR;
	msg->send_size = 0;
	reply(msg);
	switch(msg->arg[1]) {
		case REDRAW_FULL:
			FullRedraw();
			cursor_sync();
			break;
		case REDRAW_PARTIAL:
			PartialRedraw(msg->arg[2]);
			cursor_sync();
			break;
		case REDRAW_CURSOR:
			cursor_sync();
			break;
	}
	return 0;
}

void ProcessRedraw() {
#if 0
	while(1) {
		struct message msg;
		msg.flags = 0;
		msg.tid = redraw_thread;
		msg.send_size = 0;
		msg.recv_size = 0;
		receive(&msg);
		msg.arg[2] = NO_ERR;
		reply(&msg);
		printf("Redraw requested\n");
		switch(msg.arg[1]) {
			case REDRAW_FULL:
				FullRedraw();
				CursorRedraw();
				break;
			case REDRAW_PARTIAL:
				PartialRedraw(msg.arg[2]);
				CursorRedraw();	
				break;
			case REDRAW_CURSOR:
				CursorRedraw();
				break;
		}
	}
#endif
	nsi_t *interface = new nsi_t();

	interface->add(REDRAW_REQUEST, ProcessRedrawRequest);
	while (1) 
		interface->wait_message();

}

int RequestRedraw(int RedrawType, int window) {
	struct message msg;
	msg.flags = 0;
	msg.tid = redraw_thread;
	msg.send_size = 0;
	msg.recv_size = 0;
	msg.arg[0] = REDRAW_REQUEST;
	msg.arg[1] = RedrawType;
	msg.arg[2] = window;
	if(my_tid() != redraw_thread)
		send(&msg);
	else
		ProcessRedrawRequest(&msg);
	return msg.arg[2];
}


