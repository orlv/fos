/*
 * Copyright (C) 2008 Sergey Gridassov
 */

#ifndef WINDOWS_H
#define WINDOWS_H
#define REDRAW_FULL	0
#define REDRAW_PARTIAL	1
#define REDRAW_CURSOR	2
#define REDRAW_TYPES_CNT	3
int windows_init();
int RequestRedraw(int RedrawType, int window) ;
void ProcessRedraw();
#endif
