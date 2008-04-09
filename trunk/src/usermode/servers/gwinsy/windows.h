/*
 * Copyright (C) 2008 Sergey Gridassov
 */

#ifndef WINDOWS_H
#define WINDOWS_H
#include "context.h"

#define REDRAW_FULL	0
#define REDRAW_PARTIAL	1
#define REDRAW_CURSOR	2
#define REDRAW_TYPES_CNT	3

typedef struct {
	unsigned int parent;
	int x;
	int y;
	unsigned int w;
	unsigned int h;
	int visible;
	char title[64];
} win_attr_t;

int windows_init();
int RequestRedraw(int RedrawType, unsigned int window) ;
void ProcessRedraw();
unsigned int window_create(int x, int y, unsigned int w, unsigned int h, const char *title,  unsigned int parent, tid_t tid);
int window_map(void *buf, unsigned int window);
int window_get_attr(unsigned int handle, win_attr_t *buf);
int window_set_attr(unsigned int handle, const win_attr_t *buf);

void windows_handle_move(int x, int y);
extern context_t *backbuf;
#endif
