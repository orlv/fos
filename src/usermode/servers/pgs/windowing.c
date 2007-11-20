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
#include "windowing.h"
#include "list.h"
#include "cursor.h"
#include "close.h"
extern int need_cursor;

extern node *front;
extern node *back;

int refreshing = 0;
volatile int need_refresh = 0;

int last_handle = 0;

context_t *backbuf;
context_t *locate;
extern context_t screen;
int CreateWindow(int tid, int x, int y, int w, int h, char *caption, int flags);

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
	DrawRect(0, 0, screen.w, screen.h, 0xb6c2ff, backbuf);
	memset(locate->data, 0,  (screen.w  * screen.h * screen.bpp));
	PutString(screen.w - sizeof(version) * 8, screen.h - 16, version, 0, backbuf);
	PutString(screen.w - sizeof(version) * 8 - 3, screen.h - 18, version, 0xffffff, backbuf);
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
		DrawImage(p->w - 21, 5, &close_button,  p->context);
		DrawRect(p->x, p->y, p->w, p->h, p->handle, locate);
		FlushContext(p->context, p->w, p->h, p->x, p->y, 0, 0, backbuf);
	}
	}
	FlushContext(backbuf, screen.w, screen.h, 0, 0, 0, 0, &screen);
	need_cursor = 1;
	refreshing = 0;
}
#define RAND_MAX 	0x7ffffffe
#define	M	((1U<<31) -1)
#define	A	48271
#define	Q	44488		// M/A
#define	R	3399		// M%A; R < Q !!!

// FIXME: ISO C/SuS want a longer period
int seed = -1;
int rand(int limit)
{
	if(seed == -1) seed = uptime();
   unsigned long X;

    X = seed;
    X = A*(X%Q) - R * (unsigned long) (X/Q);
    if (X < 0)
	X += M;

    seed = X;
	return X & limit;
}


int CreateWindow(int tid, int x, int y, int w, int h, char *caption, int class) {

	struct window_t *win = malloc(sizeof(struct window_t));
	context_t *c = malloc(sizeof(context_t));
	char * title = malloc(strlen(caption));
	strcpy(title, caption);
	c->w = w;
	c->h = h;
	c->bpp = screen.bpp;
	c->data = RequestMemory(h  * w * screen.bpp);
	c->native_pixels = 0;
	win->handle = ++last_handle;
	//win->x = x;
	//win->y = y;
	win->x = rand(screen.w - w);
	win->y = rand(screen.h - h);
	win->w = w;
	win->h = h;
	win->context = c;
	win->title = title;
	win->tid = tid;
	insertBack(win);
	DrawRect(0, 0, w, h, 0xc3c3c3, c);

	line(1, 1, 1, h - 3, 0xffffff, c);
	line(1, 1, w - 3, 1, 0xffffff, c); 
	line(1, h - 2, w - 2, h - 2, 0x828282, c);
	line(w - 2, h - 1, w - 2, 1, 0x828282, c);
	line(0, h - 1, w - 1, h - 1, 0x000000, c);
	line(w - 1, h - 1, w - 1, 0, 0x000000, c);
	need_refresh = 1;
	return win->handle;
}

void SetFocusTo(int handle) {
//	printf("focus set to %u\n", handle);
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
