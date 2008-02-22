/*
 *         FOS Graphics System
 * Copyright (c) 2007-2008 Sergey Gridassov
 * Modifications by   Oleg Fedorov
 * Original cursor handling algorithm 
 *  by Sadovnikov Vladimir
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <fos/fos.h>
#include <fos/message.h>
#include <sched.h>
#include <list.h>
#include <mutex.h>
#include <string.h>

#include <private/types.h>
#include <private/cursor.h>
#include <private/windowing.h>
#include <private/ipc.h>

window_t *curr_window = 0;
static int last_x, last_y;
static int dragging = 0;
static int down = 0;

extern context_t screen;
extern int need_cursor;

volatile int ready_counter = 4;

struct mouse_pos {
  int dx;
  int dy;
  int dz;
  int b;
};


void mouse_thread()
{
  int oldb = 0;
  int psaux;

  do {
    psaux = open("/dev/psaux", 0);
    sched_yield();
  } while (!psaux);
  struct mouse_pos move;
  ready_counter--;
  while (1) {
    read(psaux, &move, sizeof(struct mouse_pos));
    if (move.dx || move.dy) {	// мышь сдвинули
      mousex += move.dx;
      mousey += move.dy;
      if (mousex < 0)
	mousex = 0;
      if (mousey < 0)
	mousey = 0;
      if (mousex > screen.w)
	mousex = screen.w;
      if (mousey > screen.h)
	mousey = screen.h;
      need_cursor = 1;
      int handle = get_window_handle(mousex, mousey);
      int win_x = 0, win_y = 0;
      window_t *win = NULL;
      if (handle) {
	win = GetWindowInfo(handle);
	if (!win)
	  continue;
	win_x = mousex - win->x;
	win_y = mousey - win->y;
	if (win->class & WC_MOUSEMOVE) {
	  if (win->class & WC_NODECORATIONS)
	    PostEvent(win->tid, handle, EV_MMOVE, win_x, win_y, 0, 0);
	  else {
	    if (win_x > 3 && win_x < win->w - 3 && win_y > 21 && win_y < win->h - 3)
	      PostEvent(win->tid, handle, EV_MMOVE, win_x - 3, win_y - 21, 0, 0);
	  }
	}
      }
      if (down) {
	if (handle && win) {
	  if (win->class & WC_NODECORATIONS)
	    continue;
	  if (!dragging) {
	    if ((win_x >= 3) && (win_y >= 3) && (win_y <= 18) && (win_y <= (win->w - 6))) {
	      curr_window = win;
	      last_x = mousex;
	      last_y = mousey;
	      SetFocusTo(handle);
	      dragging = 1;
	      win->x_drag = win->x;
	      win->y_drag = win->y;
	      DrawBorder(0);

	    }
	  }
	}
	if (dragging) {
	  if (curr_window) {
	    curr_window->x_drag += (mousex - last_x);
	    curr_window->y_drag += (mousey - last_y);
	    if (curr_window->x_drag < 0) {
	      mousex -= curr_window->x_drag;
	      curr_window->x_drag = 0;
	    }
	    if (curr_window->y_drag < 0) {
	      mousey -= curr_window->y_drag;
	      curr_window->y_drag = 0;
	    }
	    if (curr_window->x_drag + curr_window->w > screen.w) {
	      mousex += screen.w - (curr_window->x_drag + curr_window->w);
	      curr_window->x_drag = screen.w - curr_window->w;
	    }
	    if (curr_window->y_drag + curr_window->h > screen.h - 28) {
	      mousey += screen.h - (curr_window->y_drag + curr_window->h + 28);
	      curr_window->y_drag = screen.h - curr_window->h - 28;
	    }
	    last_x = mousex;
	    last_y = mousey;
	    DrawBorder(0);
	  }
	}

      }
 
    }
    if (move.b && !oldb) {	// кнопку нажали

      oldb = move.b;

      down = 1;
      need_cursor = 1;
      int handle = get_window_handle(mousex, mousey);

      if (!handle)
	continue;
      window_t *win = GetWindowInfo(handle);
      int win_x = mousex - win->x;
      int win_y = mousey - win->y;

      if (win->class & WC_NODECORATIONS) {
	PostEvent(win->tid, handle, EV_MDOWN, win_x, win_y, 0, 0);
      } else {
	if (win_x > 3 && win_x < win->w - 3 && win_y > 21 && win_y < win->h - 3)
	  PostEvent(win->tid, handle, EV_MDOWN, win_x - 3, win_y - 21, 0, 0);
      }

    }
    if (!move.b && oldb) {	// отпустили
      oldb = move.b;

    if (curr_window && dragging) {
      DrawBorder(-1);
      need_refresh = 1;
      need_cursor = 1;
      dragging = 0;
      curr_window = 0;
    }
    down = 0;
    int handle = get_window_handle(mousex, mousey);

    if (!handle)
     continue;
    window_t *win = GetWindowInfo(handle);
    int win_x = mousex - win->x;
    int win_y = mousey - win->y;

    if (win->class & WC_NODECORATIONS) {
      PostEvent(win->tid, handle, EV_MUP, win_x, win_y, 0, 0);
    } else {
      if ((win_x >= win->w - 21) && (win_y >= 5) && (win_x <= win->w - 5) && (win_y <= 19)) {
	SetVisible(handle, 0);
	PostEvent(win->tid, handle, EV_WINCLOSE, 0, 0, 0, 0);
	continue;
      }
      if (win_x > 3 && win_x < win->w - 3 && win_y > 21 && win_y < win->h - 3)
	PostEvent(win->tid, handle, EV_MUP, win_x - 3, win_y - 21, 0, 0);
    }
    if (!win->active) {
      SetFocusTo(handle);
      need_cursor = 1;
    }

    }
  }
}

void kb_thread()
{
  int kbd;

  do {
    kbd = open("/dev/keyboard", 0);
    sched_yield();
  } while (!kbd);
  char ch;
  int extended = 0;
  ready_counter--;
  while (1) {
    read(kbd, &ch, 1);
    if (ch == 0x01) {		// расширенный символ
      extended = 1;
      continue;
    }
    window_t *w = GetActiveWindow();

    if (w)
      PostEvent(w->tid, w->handle, EV_KEY, ch & 0xFF, extended, 0, 0);
    extended = 0;
  }
}

void StartEventHandling()
{
  thread_create((off_t) & mouse_thread);
  thread_create((off_t) & kb_thread);
  int iterations = 0;
  while(ready_counter) {
    iterations++;
    sched_yield();
  }
  printf("FGS: Event handling subsystem started up (%u iterations)\n", iterations);
}

