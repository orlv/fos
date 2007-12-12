/*
 * Portable Graphics System
 *       GUI system
 * Copyright (c) 2007 Grindars
 */
#include <gui/al.h>
#include <fos/fos.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sched.h>
#include "windowing.h"
#include "list.h"
#include "cursor.h"
//#include "close.h"
extern int need_cursor;

extern node *front;
extern node *back;

volatile int refreshing = 0;
volatile int need_refresh = 0;

int last_handle = 0;

context_t *backbuf;
context_t *locate;
extern context_t screen;
extern picture_t *close_button;
int CreateWindow(int x, int y, int tid, int w, int h, char *caption, int flags);

void init_windowing() {

	backbuf = malloc(sizeof(context_t));
	backbuf->w =  screen.w;
	backbuf->h = screen.h;
	backbuf->bpp = screen.h;
	backbuf->data = RequestMemory(screen.w  * screen.h * screen.bpp);
	backbuf->native_pixels = 0;

	locate = malloc(sizeof(context_t));
	locate->w = screen.w;
	locate->h = screen.h;
	locate->bpp = screen.bpp;
	locate->data = RequestMemory(screen.w  * screen.h * screen.bpp);
	locate->native_pixels = 1;

	need_refresh = 1;

}

int get_window_handle(int x, int y) {
	return GetPixel(x, y, locate);
}

window_t *GetWindowInfo(int handle) {
	if(front == NULL) return NULL;
	for(node *p = front; p; p = p->next) {
		window_t *win = (window_t *)p->data;
		if(win->handle == handle)
			return win;
	}
	return NULL;
}
char version[] = { "Portable Graphics System version " VERSION };
void Redraw() {

	refreshing = 1;
	DrawRect(0, 0, screen.w, screen.h, 0x003082, backbuf);
	memset(locate->data, 0,  (screen.w  * screen.h * screen.bpp));
	PutString(3, 3, version, 0, backbuf);
	PutString(0, 0, version, 0xffffff, backbuf);
	if(front != NULL) {

	for(node *n = front; n; n = n->next) {
		window_t *p = (window_t *) n->data;
		if(p->visible == 0 || p->visible == -1)
			continue;
		if(n == back) {
			p->active = 1;
			if(!p->class & WC_NODECORATIONS) {
				DrawRect(3, 3, p->w - 6, 18, 0x000082, p->context);
				PutString(4,4, p->title, 0xffffff, p->context);
			}
		}else{
			p->active = 0;
			if(!p->class & WC_NODECORATIONS) {
				DrawRect(3, 3, p->w - 6, 18, 0x808080, p->context);
				PutString(4, 4, p->title, 0xc0c0c0,  p->context);
			}
		}
		if(!p->class & WC_NODECORATIONS)
			DrawImage(p->w - 21, 5, close_button,  p->context);
		DrawRect(p->x, p->y, p->w, p->h, p->handle, locate);
		FlushContext(p->context, p->context->w, p->context->h, p->x, p->y, 0, 0, backbuf);
	}
	}
	FlushContext(backbuf, screen.w, screen.h, 0, 0, 0, 0, &screen);
	need_cursor = 1;
	refreshing = 0;
}

void WindowMapped(struct window_t *win) {
//	win->visible = 1;
	DrawRect(0, 0, win->w, win->h, 0xc3c3c3, win->context);
	if(!win->class & WC_NODECORATIONS) {
		line(1, 1, 1, win->h - 3, 0xffffff, win->context);
		line(1, 1, win->w - 3, 1, 0xffffff, win->context); 
		line(1, win->h - 2, win->w - 2, win->h - 2, 0x828282, win->context);
		line(win->w - 2, win->h - 1, win->w - 2, 1, 0x828282, win->context);
		line(0, win->h - 1, win->w - 1, win->h - 1, 0x000000, win->context);
		line(win->w - 1, win->h - 1, win->w - 1, 0, 0x000000, win->context);
	}
}
int CreateWindow(int x, int y, int tid, int w, int h, char *caption, int class) {
	SetBusy(1);
	struct window_t *win = malloc(sizeof(struct window_t));
	context_t *c = malloc(sizeof(context_t));
	char * title = malloc(strlen(caption));
	strcpy(title, caption);
	if(!class & WC_NODECORATIONS) {
		h += 21 + 3;
		w += 3 + 3;
		win->x = random() % (screen.w - w);
		win->y = random() % (screen.h - h - 28);
	} else {
		win->x = x;
		win->y = y;
	}
	c->w = w;
	c->h = h;
	c->bpp = screen.bpp;
	c->native_pixels = 0;
	win->handle = ++last_handle;
	win->w = w;
	win->h = h;
	win->context = c;
	win->title = title;
	win->tid = tid;
	win->visible = -1;
	win->class = class;
	insertBack(win);
	return win->handle;
}
void SetVisible(int handle, int visible) {
	window_t *win = GetWindowInfo(handle);
	if(!win) return;
	if(visible && win->visible == -1) {
		SetBusy(-1);
		need_cursor = 1;
	}
	if(win->visible != visible) {
		while(refreshing) sched_yield();
		win->visible = visible;
		need_refresh = 1;
	}
}
void SetFocusTo(int handle) {
	for(node *n = front; n; n = n->next) {
		window_t *win = (window_t *)n->data;
		if(win->handle == handle) {
			if(win->active) return;
			removeNode(n);
			free(n);
			insertBack(win);
			need_refresh = 1;
			return;
		}
		
	}
	
}
void DestroyWindow(int handle) {
	for(node *n = front; n; n = n->next) {
		window_t *win = (window_t *)n->data;
		if(win->handle == handle) {
			free(win->context->data);
			free(win->context);
			free(win->title);
			free(win);

			if(n == front && n == back) {
				front = NULL;
				back = NULL;
			} else
				removeNode(n);
			free(n);
			need_refresh = 1;
		}
	}
	
}
void RefreshWindow(int handle) {
	struct window_t *win = GetWindowInfo(handle);
	if(!win)
		return;
	if(win->visible == 0 || win->visible == -1) return;
	if(win->active) {
		while(refreshing) sched_yield();
		if(win->class & WC_NODECORATIONS) {
			FlushContext(win->context, win->context->w, win->context->h, win->x, win->y, 0, 0, &screen);
			FlushContext(win->context, win->context->w, win->context->h, win->x, win->y, 0, 0, backbuf);
		}else {
			FlushContext(win->context, win->context->w - 6, win->context->h - 24, win->x + 3, win->y + 21, 3, 21, &screen);
			FlushContext(win->context, win->context->w - 6, win->context->h - 24, win->x + 3, win->y + 21, 3, 21, backbuf);
		}
		need_cursor = 1;
		return;
	}
	need_refresh = 1;
}
window_t * GetActiveWindow() {
	for(node *n = front; n; n = n->next) {
		window_t *win = (window_t *)n->data;
		if(win->active)
			return win;
	}
	return NULL;
}
extern window_t *curr_window;
volatile int borderx = -1;
volatile int bordery = -1;
void DrawBorder(int reset) {
	if(reset) {
		FlushContext(backbuf, curr_window->w + 1,  curr_window->h + 1, borderx, bordery, borderx, bordery, &screen);
		borderx = -1;
		bordery = -1;
		curr_window->x = curr_window->x_drag;
		curr_window->y = curr_window->y_drag;
		return;
	}
	if(borderx != -1) 
		FlushContext(backbuf, curr_window->w + 1,  curr_window->h + 1, borderx, bordery, borderx, bordery, &screen);
	

	border(curr_window->x_drag, curr_window->y_drag, curr_window->w, curr_window->h, &screen);
	borderx = curr_window->x_drag;
	bordery = curr_window->y_drag;
}
