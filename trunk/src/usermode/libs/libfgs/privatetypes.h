/*
  Copyright (C) 2007 Serge Gridassov
 */

#ifndef PRIVATETYPES_H
#define PRIVATETYPES_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <fgs/fgs.h>

typedef struct {
  int x;
  int y;
  int w;
  int h;
  int class;
} create_win_t;

struct win_info {
  int bpp;
  int handle;
  int margin_up;
  int margin_down;
  int margin_left;
  int margin_right;
};

struct win_hndl {
  int handle;
  char *data;
  int w;
  int h;
  int bpp;
  int margin_up;
  int margin_down;
  int margin_left;
  int margin_right;
};

typedef struct {
  void *reserved[2];
  int class;
  int handle;
  int a0;
  int a1;
  int a2;
  int a3;
} event_t;

#define MAX_TITLE_LEN 64
#define WIN_CMD_CREATEWINDOW	(1 + 256)
#define WIN_CMD_DESTROYWINDOW	(2 + 256)
#define WIN_CMD_WAIT_EVENT	(3 + 256)
#define WIN_CMD_CLEANUP		(4 + 256)
#define WIN_CMD_MAPBUF		(5 + 256)
#define WIN_CMD_REFRESHWINDOW	(6 + 256)
#define WIN_CMD_SETVISIBLE	(7 + 256)
#define WIN_CMD_SCREEN_INFO	(8 + 256)
#define RED(x, bits)	((x >> (16 + 8 - bits)) & ((1 << bits) - 1))
#define GREEN(x, bits)	((x >> (8 + 8 - bits)) & ((1 << bits) - 1))
#define BLUE(x, bits)	((x >> (8 - bits)) & ((1 << bits) - 1))

#define CONTROL_BUTTON	1
#define CONTROL_STATIC	2
#define CONTROL_MENU	3

typedef struct {
  int count;
  int selected;
  struct rw *win;
  struct cntrl *control;
  char **items;
} menu_t;

typedef struct cntrl {
  struct cntrl *next;
  int class;
  int x;
  int y;
  int w;
  int h;
  char *text;
  void *win;
  int down;
  menu_t *menu;
} control_t;

typedef struct rw {
  int handle;
  int *locate;
  int w;
  int h;
  int evhandle;
  control_t *control;
  struct rw *next;
  int (*handler) (int, int, int, int, int, int);
  int menu_of;
  menu_t *menu;
} rootwindow_t;

extern rootwindow_t *head;

#define STYLE_BUTTON_NORMAL 1
#define STYLE_BUTTON_DOWN 2

static inline rootwindow_t *ResolveEventHandle(int hndl)
{
  if (!head)
    return NULL;
  for (rootwindow_t * ptr = head; ptr; ptr = ptr->next) {
    if (ptr->evhandle == hndl)
      return ptr;
  }
  return NULL;
}

static inline control_t *ResolveMouseCoord(rootwindow_t * win, int x, int y)
{
  return (control_t *) win->locate[y * win->w + x];
}

static inline void DrawLocateRect(int *locate, int x, int y, int w, int h, int c, int pitch)
{
  for (int yy = y; yy < y + h; yy++)
    for (int xx = x; xx < x + w; xx++) {
      locate[yy * pitch + xx] = c;
    }
}

void Draw3D(int x, int y, int w, int h, int handle, int style);
extern int screen_width, screen_height;

static inline rootwindow_t *InternalCreateWindow(int x, int y, int w, int h, char *title,
						 int (*handler) (int, int, int, int, int, int), int style, int menu_of,
						 menu_t * menu)
{
  rootwindow_t *rw = malloc(sizeof(rootwindow_t));
  int hndl = CreateWindow(x, y, w, h, title, style, &rw->evhandle);
  int *locatebuf = malloc(sizeof(int) * w * h);
  memset(locatebuf, 0, sizeof(int) * w * h);
  rw->handle = hndl;
  rw->locate = locatebuf;
  rw->w = w;
  rw->h = h;
  rw->control = NULL;
  rw->next = head;
  rw->handler = handler;
  rw->menu_of = menu_of;
  rw->menu = menu;
  head = rw;
  return rw;
}

#endif
