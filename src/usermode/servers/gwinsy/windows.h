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

int windows_init();
int RequestRedraw(int RedrawType, int window) ;
void ProcessRedraw();
unsigned int window_create(int x, int y, unsigned int w, unsigned int h, const char *title,  unsigned int parent, tid_t tid);

extern context_t *backbuf;
#endif
