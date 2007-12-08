/*
 * Portable Graphics System
 *       GUI system
 * Copyright (c) 2007 Grindars
 */
#include <gui/al.h>

#include "cursor.h"
#include "windowing.h"

window_t *curr_window = 0;
int last_x, last_y;
int dragging = 0;
int down = 0;
extern int need_cursor;
void event_handler(event_t *event) {

	switch(event->type) {
	case EVENT_TYPE_MOUSEMOVE:

		mousex = event->mousemove->x;
		mousey = event->mousemove->y;
		need_cursor = 1;
		if (down)
		{
		    if (!dragging)
		    {
			int handle = get_window_handle(mousex, mousey);
			if (handle)
			{
			    window_t *win = GetWindowInfo(handle);
				if(!win) break;
			    int win_x = mousex-win->x;
			    int win_y = mousey-win->y;
			    if ((win_x>=3) && (win_y>=3) && (win_y<=18) && (win_y<=(win->w-6)))
			    {
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
		
		if (dragging)
		{
		    if (curr_window)
		    {
			DrawBorder(0);
			curr_window->x_drag += (mousex-last_x);
			curr_window->y_drag += (mousey-last_y);
			last_x = mousex;
			last_y = mousey;
		    }
		}

		need_cursor = 1;
		}
		break;
	case EVENT_TYPE_MOUSEDOWN: {
		down = 1;
		need_cursor = 1;
		int handle = get_window_handle(mousex, mousey);
		if(!handle) break;
		window_t *win = GetWindowInfo(handle);
		int win_x = mousex-win->x;
		int win_y = mousey-win->y;
		if(win_x > 3 && win_x < win->w - 3 && win_y > 21 && win_y < win->h - 3)
			PostEvent(win->tid, handle, EV_MDOWN,  win_x - 3 , win_y - 21, 0, 0);

		break;
	}
	case EVENT_TYPE_MOUSEUP:
		if(curr_window && dragging) {
			DrawBorder(-1);
			need_refresh = 1;
			need_cursor = 1;
			dragging = 0;
			curr_window = 0;
		}
		down = 0;
		int handle = get_window_handle(mousex, mousey);
		if(!handle) break; 
		window_t *win = GetWindowInfo(handle);
		int win_x = mousex-win->x;
		int win_y = mousey-win->y;
		if((win_x >= win->w - 21) && (win_y >= 5) && (win_x <= win->w - 5) && (win_y <= 19)) {
			PostEvent(win->tid, handle, EV_WINCLOSE, 0, 0, 0, 0);
			break;
		}
		if(!win->active) {
			SetFocusTo(handle);
			need_cursor = 1;
		}
		break;
	}
}
