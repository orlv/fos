/*
  Copyright (C) 2007 Serge Gridassov
 */

#include <fgs/fgs.h>
#include <fgs/controls.h>
#include <stdlib.h>
#include <string.h>
#include "privatetypes.h"

void MoveFocusToControl(int win, int handle) {
  control_t *c = (control_t *) handle;
  rootwindow_t *rw = (rootwindow_t *)win;
  control_t *oldfocus = rw->focused;
  if(c) {
    if(c->class == CONTROL_STATIC || c->class == CONTROL_MENU) {
      if(c->next)
        MoveFocusToControl(win, (int) c->next);
      return;
    }
    if(c->class == CONTROL_BUTTON) {
      Draw3D(c->x, c->y, c->w, c->h, rw->handle, STYLE_BUTTON_FOCUSED);
      pstring(rw->handle, c->x + (c->w - 8 * strlen(c->text)) / 2, c->y + (c->h - 16) / 2, 0x000000, c->text);
    }
  }
  rw->focused = c;
  if(oldfocus) {
     if(oldfocus->class == CONTROL_BUTTON) {
       Draw3D(oldfocus->x, oldfocus->y, oldfocus->w, oldfocus->h, rw->handle, STYLE_BUTTON_NORMAL);
       pstring(rw->handle, oldfocus->x + (oldfocus->w - 8 * strlen(oldfocus->text)) / 2, oldfocus->y + (oldfocus->h - 16) / 2, 0x000000, oldfocus->text);
    }
  }
  RefreshWindow(rw->handle);
}
void SetControlText(int handle, char *text)
{
  control_t *c = (control_t *) handle;
  rootwindow_t *rw = c->win;

  free(c->text);
  c->text = malloc(strlen(text));
  strcpy(c->text, text);
  if (c->class == CONTROL_STATIC) {
    rect(rw->handle, c->x, c->y, c->w, c->h, 0xC3C3C3);
    pstring(rw->handle, c->x, c->y, 0x000000, c->text);
  } else {
    Draw3D(c->x, c->y, c->w, c->h, rw->handle, STYLE_BUTTON_NORMAL);
    pstring(rw->handle, c->x + (c->w - 8 * strlen(c->text)) / 2, c->y + (c->h - 16) / 2, 0x000000, c->text);
  }
  RefreshWindow(rw->handle);
}

int CreateButton(int window, int x, int y, int w, int h, char *caption)
{
  int len = strlen(caption);
  rootwindow_t *rw = (rootwindow_t *) window;
  control_t *c = malloc(sizeof(control_t));

  c->class = CONTROL_BUTTON;
  c->x = x;
  c->y = y;
  c->w = w;
  c->h = h;
  c->text = malloc(len);
  strcpy(c->text, caption);
  c->next = rw->control;
  c->win = rw;
  c->down = 0;
  c->menu = NULL;
  rw->control = c;
  DrawLocateRect(rw->locate, x, y, w, h, (int)c, rw->w);
  Draw3D(x, y, w, h, rw->handle, STYLE_BUTTON_NORMAL);
  pstring(rw->handle, (w - 8 * len) / 2 + x, (h - 16) / 2 + y, 0x000000, caption);
  RefreshWindow(rw->handle);
  return (int)c;
}

int CreateStatic(int window, int x, int y, int w, int h, char *caption)
{
  int len = strlen(caption);
  rootwindow_t *rw = (rootwindow_t *) window;
  control_t *c = malloc(sizeof(control_t));

  c->class = CONTROL_STATIC;
  c->x = x;
  c->y = y;
  c->w = w;
  c->h = h;
  c->text = malloc(len);
  strcpy(c->text, caption);
  c->next = rw->control;
  c->win = rw;
  c->menu = NULL;
  rw->control = c;
  rect(rw->handle, x, y, w, h, 0xC3C3C3);
  pstring(rw->handle, x, y, 0x000000, caption);
  RefreshWindow(rw->handle);
  return (int)c;
}

void DestroyControl(int handle)
{
  control_t *cntrl = (control_t *) handle;
  rootwindow_t *rw = cntrl->win;
  for (control_t * ptr = rw->control, *prev = NULL; ptr; prev = ptr, ptr = ptr->next) {
    if (ptr == cntrl && prev) {
      prev->next = ptr->next;
      break;
    }
    if (ptr == cntrl && ptr == rw->control) {
      rw->control = ptr->next;
      break;
    }
  }
  if((int)rw->focused == handle) MoveFocusToControl((int)rw, (int)rw->control);
  if (cntrl->class == CONTROL_BUTTON)
    DrawLocateRect(rw->locate, cntrl->x, cntrl->y, cntrl->w, cntrl->h, 0, rw->w);
  if (cntrl->class == CONTROL_MENU) {
    DestroyControlsWindow((int)cntrl->menu->win);
    free(cntrl->menu->items);
    free(cntrl);
  } else {
    free(cntrl->text);
    free(cntrl);
    rect(rw->handle, cntrl->x, cntrl->y, cntrl->w, cntrl->h, 0xc3c3c3);
    RefreshWindow(rw->handle);
  }
}

int MenuEventHandler(int hwnd, int class, int a0, int a1, int a2, int a3)
{
  rootwindow_t *win = (rootwindow_t *) hwnd;
  rootwindow_t *parent = (rootwindow_t *) win->menu_of;

  if (class != EVC_CLICK)
    return 1;
  if (a0 < 4 || a1 < 4 || a0 > win->w - 4 || a1 > win->h - 4)
    return 1;
  int item = a1 / 20;

  if (item + 1 > win->menu->count)
    return 1;
  (parent->handler) ((int)win->menu->control, EVC_MENU, item, 0, 0, 0);
  return 1;
}

int CreateMenu(int hndl, int x, int y, int count, char *items[])
{
  rootwindow_t *rw = (rootwindow_t *) hndl;
  int scr_width, scr_height;
  int maxlen = 0, total = 1;

  ScreenInfo(&scr_width, &scr_height);
  for (int i = 0; i < count; i++) {
    int len = strlen(items[i]);

    total += len + 1;
    if (len > maxlen)
      maxlen = len;
  }
  char **itms = malloc(total);

  memcpy(itms, items, total);
  menu_t *menudt = malloc(sizeof(menu_t));
  control_t *c = malloc(sizeof(control_t));

  c->class = CONTROL_MENU;
  c->x = 0;
  c->y = 0;
  c->w = 0;
  c->h = 0;
  c->text = NULL;
  c->next = rw->control;
  c->win = rw;
  c->menu = menudt;
  menudt->count = count;
  menudt->selected = 0;
  menudt->items = itms;
  int width = maxlen * 8 + 12;
  int height = count * 20 + 7;

  if (x + width >= scr_width)
    x -= width;

  if (y + height >= scr_height)
    y -= height;
  rootwindow_t *menu =
      InternalCreateWindow(x, y, width, height, "menu", MenuEventHandler, WC_NODECORATIONS | WC_MOUSEMOVE, hndl,
			   menudt);
  menudt->win = menu;
  menudt->control = c;
  int drawing = GetDrawingHandle((int)menu);

  line(drawing, 0, 0, width - 2, 0, 0xD8D8D8);
  line(drawing, 1, 1, width - 3, 1, 0xF8F8F8);
  line(drawing, 0, 0, 0, height, 0xD8D8D8);
  line(drawing, 1, 1, 1, height - 2, 0xF8F8F8);
  line(drawing, width - 1, 0, width - 1, height, 0x000000);
  line(drawing, width - 2, 1, width - 2, height - 1, 0x787878);
  line(drawing, 0, height - 1, width - 1, height - 1, 0x000000);
  line(drawing, 1, height - 2, width - 2, height - 2, 0x787878);
  for (int i = 0; i < count; i++) {
    pstring(drawing, 3, 6 + i * 20, 0x000000, items[i]);
  }
  ControlsWindowVisible((int)menu, 1);
  return (int)c;
}
