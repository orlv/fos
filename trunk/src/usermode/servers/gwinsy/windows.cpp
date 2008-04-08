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
#include <c++/list.h>
#include <stdio.h>
#include <mutex.h>
#include "windows.h"
#include "context.h"
#include "video.h"
#include "assert.h"
#include "cursor.h"


#define REDRAW_REQUEST	(BASE_METHOD_N + 0)

static context_t *backbuf;
static context_t *locate;
static tid_t redraw_thread;

typedef struct win {
	int	x;
	int	y;
	unsigned int	w;
	unsigned int	h;
	unsigned int	handle;
	context_t *context;
	tid_t	tid;	
	bool	visible;	
	char	*title;
	bool	focused;
	List	<struct win *> *childs;
} window_t;

typedef struct {
	unsigned int	hndl;
	List	<window_t *> *win;
} lookup_t;

static List <window_t *> *winlist;
static List <lookup_t *> *lookup_list;

static mutex_t m_winlist = 0;
static mutex_t m_lookup = 0;

static List <window_t *> *lookup(unsigned int hndl) {
	List <lookup_t *> * ptr = NULL;
	while(!mutex_try_lock(&m_lookup))
		sched_yield();

	list_for_each(ptr, lookup_list) {
		if(ptr->item->hndl == hndl) {
			mutex_unlock(&m_lookup);
			return ptr->item->win;
		}
	}
	mutex_unlock(&m_lookup);
	return NULL;
}

int windows_init() {
	winlist = new List <window_t *>;
	lookup_list = new List <lookup_t *>;

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
