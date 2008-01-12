/*
  Copyright (C) 2007 Serge Gridassov
 */

#include <stdio.h>
#include <string.h>
#include <fgs/fgs.h>
#include <fgs/controls.h>
#include "privatetypes.h"

void ControlsMessageLoop()
{
  int class, handle, a0, a1, a2, a3;
  rootwindow_t *win;
  control_t *control;

  while (1) {
    WaitEvent(&class, &handle, &a0, &a1, &a2, &a3);
    win = ResolveEventHandle(handle);
    if (!win) {
      printf("Warning: can't handle event for window %u\n", handle);
      continue;
    }
    switch (class) {
    case EV_WINCLOSE:
      if (!(win->handler) ((int)win, class, a0, a1, a2, a3)) 
	return;

      break;
    case EV_MDOWN:
      control = ResolveMouseCoord(win, a0, a1);
      if (!control) {
        MoveFocusToControl((int) win, 0);
	break;
      }
      MoveFocusToControl((int) win, (int)control);
      if (control->class == CONTROL_BUTTON) {
	control->down = 1;
	__InternalRedrawControl(control, 0, 0);
      }
      RefreshWindow(win->handle);
      break;
    case EV_MUP:
      if (win->menu_of) {
	(win->handler) ((int)win, EVC_CLICK, a0, a1, 0, 0);
	break;
      }
      for (control_t * ptr = win->control; ptr; ptr = ptr->next) {
	if (ptr->down) {
	  ptr->down = 0;
	  if (ptr->class == CONTROL_BUTTON) {
            __InternalRedrawControl(ptr, 0, 0);
	    RefreshWindow(win->handle);
	  }
	}
      }
      control = ResolveMouseCoord(win, a0, a1);
      if (!control)
	break;
      if (control->class == CONTROL_BUTTON) {
	if (!(win->handler) ((int)control, EVC_CLICK, a0 - control->x, a1 - control->y, 0, 0))
	  return;
      }
      break;
    case EV_MMOVE:
      if (win->menu_of) {
        __InternalRedrawControl(win->menu->control, a0, a1);
	RefreshWindow(win->handle);
      }
      break;
    case EV_KEY:
      if(a0 == '\t') {
        if(win->focused) {
          if(win->focused->next)
            MoveFocusToControl((int) win, (int)win->focused->next);
          else
            MoveFocusToControl((int) win, (int)win->control);
        } else
          MoveFocusToControl((int) win, (int)win->control);
        break;
      }
      if(a0 == '\n') {
        if(win->focused) {
          if(win->focused->class == CONTROL_BUTTON) {
            if (!(win->handler) ((int)win->focused, EVC_CLICK, 0, 0, 0, 0))
	      return;
            break;
          }
        }
      }
      break;
    case EV_NEWWIN:
    case EV_DESTROYWIN:
      (win->handler) ((int)win, class, a0, a1, a2, a3);
      break;
    default:
      printf("Event %u(%u, %u, %u, %u) for window at ptr 0x%x\n", class, a0, a1, a2, a3, win);
    }
  }
}
