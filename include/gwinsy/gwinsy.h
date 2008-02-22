/*
 * include/gwinsy/gwinsy.h
 * Функции GUI
 *
 * Copyright (C) 2007 Serge Gridassov
 */
 
#ifndef _GWINSY_GWINSY_H
#define _GWINSY_GWINSY_H

#include <types.h>

#define MAX_TITLE_LEN 64

#define WC_WINDOW		0
#define WC_MOUSEMOVE		2
#define WC_NODECORATIONS	1
#define WC_WINDOWSEVENTS	4
#define WC_CENTERED		8
#define EV_WINCLOSE 	1
#define EV_MDOWN 	2
#define EV_MUP 		3
#define EV_KEY	 	4
#define EV_MMOVE 	5
#define EV_NEWWIN	6
#define EV_DESTROYWIN	7

asmlinkage void GUIInit();
asmlinkage int CreateWindow(int x, int y, int w, int h, char *caption, int flags, int *evhndl);
asmlinkage void WaitEvent(int *type, int *handle, int *a0, int *a1, int *a2, int *a3) ;
asmlinkage void DestroyWindow(int handle) ;
asmlinkage void GuiEnd();
asmlinkage void pixel(int handle, int x, int y, int color);
asmlinkage void RefreshWindow(int handle);
asmlinkage void rect(int handle, int x, int y, int width, int height, int color);
asmlinkage void line(int handle, int x0, int y0, int x1, int y1, int color);
asmlinkage void pstring(int handle, int x, int y, int color, char *str);
asmlinkage void SetVisible(int handle, int visible);
asmlinkage void ScreenInfo(int *width, int *height);
asmlinkage void ShiftWindowUp(int handle, int count);
asmlinkage void SetFocus(int handle);
asmlinkage int GetWindowTitle(int handle, char *buf);

#endif
