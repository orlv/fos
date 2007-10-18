/*
 * Portable Graphics System
 *       GUI system
 * Copyright (c) 2007 Grindars
 */
#include <gui/al.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "windowing.h"
#include "list.h"
#include "cursor.h"
#include "version.h"

extern mode_definition_t mode;

extern node *front;
extern node *back;

int refreshing = 0;
int need_refresh = 0;

int last_handle = 0;

context_t *backbuf;
context_t *locate;

void CreateWindow(int x, int y, int w, int h, char *caption);

void init_windowing() {

	char *backbuf_mem = RequestMemory(mode.width  * mode.height * mode.bpp);
	backbuf = malloc(sizeof(context_t));
	backbuf->w =  mode.width;
	backbuf->h = mode.height;
	backbuf->bpp = mode.bpp;
	backbuf->data = backbuf_mem;
	backbuf->native_pixels = 0;

	char *locate_mem = RequestMemory(mode.width  * mode.height * mode.bpp);
	locate = malloc(sizeof(context_t));
	locate->w = mode.width;
	locate->h = mode.height;
	locate->bpp = mode.bpp;
	locate->data = locate_mem;
	backbuf->native_pixels = 1;

	DrawRect(0, 0, mode.width, mode.height, 0, locate);

	need_refresh = 1;

	CreateWindow(100, 100, 200, 200, "Test window");

	CreateWindow(250, 150, 200, 200, "Test window 2");

	CreateWindow(350, 200, 200, 200, "Test window 3");

	CreateWindow(450, 250, 200, 200, "Test window 4");
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
	memset(backbuf->data, 0xCC, mode.width  * mode.height * mode.bpp);
	memset(locate->data, 0,  (mode.width  * mode.height * mode.bpp));
	PutString(mode.width - sizeof(version) * 8, mode.height - 16, version, 0, backbuf);
	PutString(mode.width - sizeof(version) * 8 - 3, mode.height - 18, version, 0xffffff, backbuf);
	if(front != NULL) {

	for(node *n = front; n; n = n->next) {
		window_t *p = (window_t *) n->data;
		if(n == back) {
			p->active = 1;
			DrawRect(3, 3, p->w - 6, 18, 0x000082, p->context);
			PutString(4,4, p->title, 0xffffff, p->context);
		}else{
			p->active = 0;
			DrawRect(3, 3, p->w - 6, 18, 0x808080, p->context);
			PutString(4, 4, p->title, 0xc0c0c0,  p->context);
		}
		DrawRect(p->x, p->y, p->w, p->h, p->handle, locate);
		FlushContext(p->context, p->w, p->h, p->x, p->y, 0, 0, backbuf);
	}
	}
	line(0, 0, mode.width, mode.height, 0xFF0000, backbuf);
	FlushBackBuffer(backbuf->data);
	refreshing = 0;
}

void CreateWindow(int x, int y, int w, int h, char *caption) {
	char *video = RequestMemory(h * w * mode.bpp);
	struct window_t *win = malloc(sizeof(struct window_t));
	context_t *c = malloc(sizeof(context_t));
	char * title = malloc(strlen(caption));
	strcpy(title, caption);
	c->w = w + 1;
	c->h = h + 1;
	c->bpp = mode.bpp;
	c->data = video;
	c->native_pixels = 0;
	win->handle = ++last_handle;
	win->x = x;
	win->y = y;
	win->w = w;
	win->h = h;
	win->context = c;
	win->title = title;
	insertBack(win);
	DrawRect(0, 0, w, h, 0xc3c3c3, c);
	line(1, 1, 1, h - 2, 0xffffff, c);
	line(1, 1, w - 2, 1, 0xffffff, c); 
	line(1, h - 1, w - 1, h - 1, 0x828282, c);
	line(w - 1, h - 1, w - 1, 1, 0x828282, c);
	line(0, h, w, h, 0x000000, c);
	line(0, h, w, h, 0x000000, c);
	PutString(4,4, caption, 0xffffff, c);
	need_refresh = 1;
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
