/*
 * Portable Graphics System
 *       GUI system
 * Copyright (c) 2007 Grindars
 */
#ifndef __WINDOWING_H
#define __WINDOWING_H

typedef struct window_t {
	int x;
	int y;
	int w;
	int h;
	int handle;
	int active;
	context_t *context;
	char *title;
} window_t;

extern int _down;
extern int need_refresh;

void init_windowing();
void HandleMouseClick(int x, int y, int down);
void SetFocusTo(int handle);
void Redraw();
void HandleDragAndDrop(int x, int y);

int get_window_handle(int x, int y);
window_t *GetWindowInfo(int handle);
#endif
