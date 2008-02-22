/*
 *         FOS Graphics System
 * Copyright (c) 2007 Sergey Gridassov
 */


#include <fos/fos.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sched.h>
#include <sys/mman.h>
#include <private/windowing.h>
#include <private/wlist.h>
#include <private/cursor.h>
#include <private/types.h>
#include <private/pixel.h>
#include <private/picture.h>
#include <private/context.h>
#include <private/events.h>

extern int need_cursor;

extern node *front;
extern node *back;

volatile int refreshing = 0;
volatile int need_refresh = 0;

int last_handle = 0;

context_t *backbuf;
context_t *locate;
extern context_t screen;
extern picture_t *close_button;

mutex_t winlist_mutex = 0;


void init_windowing()
{

  backbuf = malloc(sizeof(context_t));
  backbuf->w = screen.w;
  backbuf->h = screen.h;
  backbuf->bpp = screen.h;
  backbuf->data = kmmap(0, screen.w * screen.h * screen.bpp, 0, 0);
  backbuf->native_pixels = 0;

  locate = malloc(sizeof(context_t));
  locate->w = screen.w;
  locate->h = screen.h;
  locate->bpp = screen.bpp;
  locate->data = kmmap(0, screen.w * screen.h * screen.bpp, 0, 0);
  locate->native_pixels = 1;

  need_refresh = 1;
  printf("FGS: windowing subsystem started, allocated ~%u Kb of memory\n", ((screen.w * screen.h * screen.bpp) * 2 + sizeof(context_t) * 2) / 1024);

}

int get_window_handle(int x, int y)
{
  return GetPixel(x, y, locate);
}

window_t *GetWindowInfo(int handle)
{
  if (front == NULL)
    return NULL;
  while (!mutex_try_lock(winlist_mutex))
    sched_yield();
  for (node * p = front; p; p = p->next) {
    window_t *win = (window_t *) p->data;

    if (win->handle == handle) {
      mutex_unlock(winlist_mutex);
      return win;
    }
  }
  mutex_unlock(winlist_mutex);
  return NULL;
}

void Redraw()
{

  refreshing = 1;
  DrawRect(0, 0, screen.w, screen.h, 0x003082, backbuf);
  PutString(0, 0, "FOS Graphics System version " VERSION " builded at " __DATE__ " " __TIME__ , 0xFFFFFF, backbuf);
  PutString(0, 16, "FOS - a world domination project" , 0xFFFFFF, backbuf);
  memset(locate->data, 0, (screen.w * screen.h * screen.bpp));
  if (front != NULL) {
    while (!mutex_try_lock(winlist_mutex))
      sched_yield();
    for (node * n = front; n; n = n->next) {
      window_t *p = (window_t *) n->data;

      if (p->visible < 0)
	continue;
      if (n == back) {
	p->active = 1;
	if (!(p->class & WC_NODECORATIONS)) {
	  DrawRect(3, 3, p->w - 6, 18, 0x000082, p->context);
	  PutString(4, 4, p->title, 0xffffff, p->context);
	}
      } else {
	p->active = 0;
	if (!(p->class & WC_NODECORATIONS)) {
	  DrawRect(3, 3, p->w - 6, 18, 0x808080, p->context);
	  PutString(4, 4, p->title, 0xc0c0c0, p->context);
	}
      }
      if (!(p->class & WC_NODECORATIONS))
	DrawImage(p->w - 21, 5, close_button, p->context);
      DrawRect(p->x, p->y, p->w, p->h, p->handle, locate);
      FlushContext(p->context, p->context->w, p->context->h, p->x, p->y, 0, 0, backbuf);
    }
    mutex_unlock(winlist_mutex);
  }
  FlushContext(backbuf, screen.w, screen.h, 0, 0, 0, 0, &screen);
  need_cursor = 1;
  refreshing = 0;
}

void WindowMapped(struct window_t *win)
{
  DrawRect(0, 0, win->w, win->h, 0xc3c3c3, win->context);
  if (!(win->class & WC_NODECORATIONS)) {
    line(1, 1, 1, win->h - 3, 0xffffff, win->context);
    line(1, 1, win->w - 3, 1, 0xffffff, win->context);
    line(1, win->h - 2, win->w - 2, win->h - 2, 0x828282, win->context);
    line(win->w - 2, win->h - 1, win->w - 2, 1, 0x828282, win->context);
    line(0, win->h - 1, win->w - 1, win->h - 1, 0x000000, win->context);
    line(win->w - 1, win->h - 1, win->w - 1, 0, 0x000000, win->context);
    for (node * n = front; n; n = n->next) {
      window_t *w = (window_t *) n->data;
      if((w->class & WC_WINDOWSEVENTS) && w->handle != win->handle)
        PostEvent(w->tid, w->handle, EV_NEWWIN, win->handle, 0, 0, 0);
    }
  }
  mutex_unlock(winlist_mutex);
}

int CreateWindow(int x, int y, int tid, int w, int h, char *caption, int class)
{
  SetBusy(1);
  struct window_t *win = malloc(sizeof(struct window_t));
  context_t *c = malloc(sizeof(context_t));
  char *title = malloc(strlen(caption));

  strcpy(title, caption);
  if(class & WC_NODECORATIONS) {
    win->x = x;
    win->y = y;
  } else if(class & WC_CENTERED) {
    h += 21 + 3;
    w += 3 + 3;
    win->x = (screen.w - w) / 2;
    win->y = (screen.h - h) / 2;
  } else {
    h += 21 + 3;
    w += 3 + 3;
    win->x = (unsigned long int)random() % (screen.w - w);
    win->y = (unsigned long int)random() % (screen.h - h - 28);
  }
  c->w = w;
  c->h = h;
  c->bpp = screen.bpp;
  c->native_pixels = 0;
  win->handle = ++last_handle;
  win->w = w;
  win->h = h;
  win->context = c;
  win->title = title;
  win->tid = tid;
  win->visible = -1;
  win->class = class;
  while (!mutex_try_lock(winlist_mutex))
    sched_yield();
  insertBack(win);
  mutex_unlock(winlist_mutex);
  return win->handle;
}

void SetVisible(int handle, int visible)
{
  window_t *win = GetWindowInfo(handle);

  if (!win)
    return;
  if (visible && win->visible == -1) {
    SetBusy(-1);
    need_cursor = 1;
  }
  if (win->visible != visible) {
    while (refreshing)
      sched_yield();
    win->visible = visible;
    need_refresh = 1;
  }
}

void SetFocusTo(int handle)
{
  while (!mutex_try_lock(winlist_mutex))
    sched_yield();
  for (node * n = front; n; n = n->next) {
    window_t *win = (window_t *) n->data;

    if (win->handle == handle) {
      if (win->active) {
        mutex_unlock(winlist_mutex);
	return;
      }
      removeNode(n);
      free(n);
      insertBack(win);
      need_refresh = 1;
      mutex_unlock(winlist_mutex);
      return;
    }

  }
  mutex_unlock(winlist_mutex);

}

void DestroyWindow(int handle)
{
  while (!mutex_try_lock(winlist_mutex))
    sched_yield();
  for (node * n = front; n; n = n->next) {
    window_t *win = (window_t *) n->data;

    if (win->handle == handle) {
      int needev = !(win->class & WC_NODECORATIONS);
      free(win->context->data);
      free(win->context);
      free(win->title);
      free(win);

      if (n == front && n == back) {
	front = NULL;
	back = NULL;
      } else
	removeNode(n);
      free(n);
      need_refresh = 1;
      if(needev) {
        for (node * n = front; n; n = n->next) {
          window_t *w = (window_t *) n->data;
          if((w->class & WC_WINDOWSEVENTS))
            PostEvent(w->tid, w->handle, EV_DESTROYWIN, handle, 0, 0, 0);
        }
      }
      break;
     }
   }
  mutex_unlock(winlist_mutex);
}

void RefreshWindow(int handle)
{
  struct window_t *win = GetWindowInfo(handle);

  if (!win)
    return;
  if (win->visible == 0 || win->visible == -1)
    return;
  if (win->active) {
    while (!mutex_try_lock(winlist_mutex))
      sched_yield();
    if (win->class & WC_NODECORATIONS) {
      FlushContext(win->context, win->context->w, win->context->h, win->x, win->y, 0, 0, &screen);
      FlushContext(win->context, win->context->w, win->context->h, win->x, win->y, 0, 0, backbuf);
    } else {
      FlushContext(win->context, win->context->w - 6, win->context->h - 24, win->x + 3, win->y + 21, 3, 21, &screen);
      FlushContext(win->context, win->context->w - 6, win->context->h - 24, win->x + 3, win->y + 21, 3, 21, backbuf);
    }
    need_cursor = 1;
    mutex_unlock(winlist_mutex);
  } else
    need_refresh = 1;
}

window_t *GetActiveWindow()
{
  while (!mutex_try_lock(winlist_mutex))
    sched_yield();
  for (node * n = front; n; n = n->next) {
    window_t *win = (window_t *) n->data;

    if (win->active) {
      mutex_unlock(winlist_mutex);
      return win;
    }
  }
  mutex_unlock(winlist_mutex);
  return NULL;
}

extern window_t *curr_window;
volatile int borderx = -1;
volatile int bordery = -1;

void DrawBorder(int reset)
{
  while (!mutex_try_lock(winlist_mutex))
    sched_yield();
  if (reset) {
    FlushContext(backbuf, curr_window->w + 1, curr_window->h + 1, borderx, bordery, borderx, bordery, &screen);
    borderx = -1;
    bordery = -1;
    curr_window->x = curr_window->x_drag;
    curr_window->y = curr_window->y_drag;
    return;
  }
  if (borderx != -1) {
    FlushContext(backbuf, curr_window->w + 1, 1, borderx, bordery, borderx, bordery, &screen);
    FlushContext(backbuf, curr_window->w + 1, 1, borderx, bordery  + curr_window->h - 1, borderx, bordery  + curr_window->h - 1, &screen);
    FlushContext(backbuf, 1, curr_window->h + 1, borderx, bordery, borderx, bordery, &screen);
    FlushContext(backbuf, 1, curr_window->h + 1, borderx + curr_window->w - 1, bordery, borderx + curr_window->w - 1, bordery, &screen);
  }

  border(curr_window->x_drag, curr_window->y_drag, curr_window->w, curr_window->h, &screen);
  borderx = curr_window->x_drag;
  bordery = curr_window->y_drag;
  mutex_unlock(winlist_mutex);
}

int GetWindowTitle(int handle, char *buf) {
	window_t *win = GetWindowInfo(handle);
	if(!win)
		return -1;
	strcpy(buf, win->title);
	return 0;
}
