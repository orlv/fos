/*
 * include/gwinsy/controls.h
 * Copyright (C) 2007 Serge Gridassov
 */

#ifndef _GWINSY_CONTROLS_H
#define _GWINSY_CONTROLS_H

#include <types.h>

#define EVC_CLICK	0xF0000001
#define EVC_MENU	0xF0000002

asmlinkage int CreateControlsWindow(int x, int y, int w, int h, char *title,  int (*handler)(int, int, int, int, int, int), int style);
asmlinkage void DestroyControlsWindow(int hndl);
asmlinkage void ControlsMessageLoop();
asmlinkage int CreateButton(int window, int x, int y, int w, int h, char *caption);
asmlinkage void DestroyControl(int handle);
asmlinkage void ControlsWindowVisible(int handle, int visible);
asmlinkage int CreateStatic(int window, int x, int y, int w, int h, char *caption);
asmlinkage void SetControlText(int handle, char *text);
asmlinkage int GetDrawingHandle(int handle);
asmlinkage int CreateMenu(int hndl, int x, int y, int count, char *items[]);
asmlinkage void MoveFocusToControl(int win, int handle);

#endif
