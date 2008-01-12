/*
 *         FOS Graphics System
 * Copyright (c) 2007 Sergey Gridassov
 */


#ifndef __WINDOWING_H
#define __WINDOWING_H
#include <private/types.h>
typedef struct window_t {
  unsigned int x;
  unsigned int y;
  int x_drag;
  int y_drag;
  unsigned int w;
  unsigned  int h;
  int handle;
  unsigned int active;
  context_t *context;
  char *title;
  unsigned int tid;
  unsigned int visible;
  unsigned int class;
} window_t;

extern int _down;
extern volatile int need_refresh;

void init_windowing();
void HandleMouseClick(int x, int y, int down);
void SetFocusTo(int handle);
void Redraw();
void HandleDragAndDrop(int x, int y);

int get_window_handle(int x, int y);
window_t *GetWindowInfo(int handle);
void DrawBorder(int reset);
int CreateWindow(int x, int y, int tid, int w, int h, char *caption, int flags);
void DestroyWindow(int handle);
void SetVisible(int handle, int visible);
void RefreshWindow(int handle);
window_t *GetActiveWindow();
void WindowMapped(struct window_t *win);
int GetWindowTitle(int handle, char *buf);
#endif
