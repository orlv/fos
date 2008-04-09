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
#include "ipc.h"

#define REDRAW_REQUEST	(BASE_METHOD_N + 0)

context_t *backbuf;
static context_t *locate;
static tid_t redraw_thread;

typedef struct win {
	int	x;
	int	y;
	int	absolute_x;
	int	absolute_y;
	unsigned int	w;
	unsigned int	h;
	unsigned int	handle;
	context_t *context;
	tid_t	tid;	
	bool	visible;	
	char	*title;
	bool	focused;
	unsigned int parent_handle;
	List	<struct win *> *parent;
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

static List <window_t *> *lookup(unsigned int hndl, bool unlocked = false) {
	List <lookup_t *> * ptr = NULL;

	if(!unlocked) while(!mutex_try_lock(&m_lookup))
		sched_yield();

	list_for_each(ptr, lookup_list) {
		if(ptr->item->hndl == hndl) {
			if(!unlocked) mutex_unlock(&m_lookup);
			return ptr->item->win;
		}
	}
	if(!unlocked) mutex_unlock(&m_lookup);
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
	locate->native_pixels = 1;
	locate->data = kmmap(0, screen_bytes, 0, 0);

	assert(locate->data != NULL);

	redraw_thread = my_tid();

	RequestRedraw(REDRAW_FULL, 0);

	return 0;
}

static void RedrawList(List <window_t *> *list) {
	List <window_t *> *ptr;
	printf("Redrawing list %x\n", list);
	list_for_each_prev(ptr, list) {
		window_t *win = ptr->item;
		if(win->visible) {
			Blit(win->context, backbuf, win->absolute_x, win->absolute_y, win->w, win->h, 0, 0);

			Rect(win->absolute_x, win->absolute_y, win->w, win->h, win->handle, locate);

			RedrawList(win->childs); // превед рекурсия
		}
	}
}

static void FullRedraw() {
	Rect(0, 0, backbuf->w, backbuf->h, 0x888888, backbuf);
	Rect(0, 0, locate->w, locate->h, 0, locate);

	while(!mutex_try_lock(&m_winlist))
		sched_yield();

	RedrawList(winlist);

	mutex_unlock(&m_winlist);

	Blit(backbuf, screen, 0, 0, backbuf->w, backbuf->h, 0, 0);
}

static void PartialRedraw(unsigned int hndl) {

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

int RequestRedraw(int RedrawType, unsigned int window) {
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

unsigned int last_handle = 0;

unsigned int window_create(int x, int y, unsigned int w, unsigned int h, const char *title,  unsigned int parent, tid_t tid) {
	window_t *parent_data = NULL;
	List <window_t *> *parent_item = winlist;
	if(parent) {
		parent_item = lookup(parent);
		if(!parent_item)
			return 0;
		
		parent_data = parent_item->item;
	}

	while(!mutex_try_lock(&m_lookup))
		sched_yield();

	while(!mutex_try_lock(&m_winlist))
		sched_yield();

	context_t *context = new context_t;

	assert(context != NULL);

	context->w = w;
	context->h = h;
	context->bpp = screen->bpp;
	context->native_pixels = 0;
	context->data = NULL;

	window_t *win = new window_t;

	assert(win != NULL);

	win->x = x;
	win->y = y;
	win->absolute_x = parent ? parent_data->absolute_x + x: x;
	win->absolute_y = parent ? parent_data->absolute_y + y: y;

	win->w = w;
	win->h = h;
	win->handle = ++last_handle;
	win->context = context;
	win->tid = tid;
	win->visible = false;
	win->title = strdup(title);
	win->focused = false;
	win->childs = new List<window_t *>();
	win->parent = parent_item;
	win->parent_handle = parent;
	lookup_t * lookup = new lookup_t;

	assert(lookup != NULL);

	if(parent_data) 
		lookup->win = parent_data->childs->add(win);
	else
		lookup->win = winlist->add(win);

	assert(lookup->win != NULL);

	lookup->hndl = win->handle;

	assert(lookup_list->add(lookup) != NULL);

	mutex_unlock(&m_winlist);
	mutex_unlock(&m_lookup);
	return win->handle;
} 

int window_map(void *buf, unsigned int window) {
	List <window_t *> *item = lookup(window);
	if(item == NULL)
		return -1;

	while(!mutex_try_lock(&m_winlist))
		sched_yield();

	item->item->context->data = buf;

	printf("Mapped buffer 0x%X to window %u\n", buf, window);

	if(item->item->visible) RequestRedraw(REDRAW_FULL, 0);

	mutex_unlock(&m_winlist);

	return 0;
}

int window_get_attr(unsigned int handle, win_attr_t *buf) {
	List <window_t *> *item = lookup(handle);
	if(item == NULL)
		return -1;

	while(!mutex_try_lock(&m_winlist))
		sched_yield();

	window_t *win = item->item;

	buf->parent = win->parent_handle;
	buf->x = win->x;
	buf->y = win->y;
	buf->w = win->w;
	buf->h = win->h;
	strncpy(buf->title, win->title, 64);

	mutex_unlock(&m_winlist);

	return 0;
}

int window_set_attr(unsigned int handle, const win_attr_t *buf) {
	int need_redraw = 0;

	List <window_t *> *item = lookup(handle);

	if(item == NULL)
		return -1;

	while(!mutex_try_lock(&m_winlist))
		sched_yield();

	window_t *win = item->item;
	
	if(buf->parent != win->parent_handle) {
		printf("Re-parenting started\n");
		List <window_t *> *new_parent = lookup(buf->parent, true);
		if(new_parent == NULL) {
			printf("Re-parenting failed\n");
			mutex_unlock(&m_winlist);
			return -1;
		}

		item->move(new_parent->item->childs);

		printf("Re-parenting finished\n");
		
	}

	if(win->visible || buf->visible != win->visible) need_redraw = 1;

	win->x = buf->x;
	win->y = buf->y;
	win->visible = buf->visible;
	delete win->title;
	win->title = strdup(buf->title);	

	win->absolute_x = win->parent_handle ? win->parent->item->absolute_x + win->x: win->x;
	win->absolute_y = win->parent_handle ? win->parent->item->absolute_y + win->y: win->y;

	mutex_unlock(&m_winlist);

	if(need_redraw) RequestRedraw(REDRAW_FULL, 0);

	PostEvent(0, 0, EV_GLOBAL, EVG_ATTR_MODIFY, handle, 0, 0, 0);

	return 0;
}

void windows_handle_move(int x, int y) {
	if(!locate || !locate->data)
		return;

	unsigned int handle = GetPixel(x, y, locate);
	if(!handle)
		return;

	List <window_t *> *win = lookup(handle);
	assert(win != NULL);

	while(!mutex_try_lock(&m_winlist))
		sched_yield();

	PostEvent(win->item->tid, win->item->handle, EV_MMOVE, 0, x, y, 0, 0);

	mutex_unlock(&m_winlist);
}
