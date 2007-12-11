/*
 * Portable Graphics System
 *       GUI system
 * Copyright (c) 2007 Grindars
 */
#include <stddef.h>
#include <gui/al.h>

#include "cursor.h"
#include "windowing.h"

window_t *curr_window = 0;
int last_x, last_y;
int dragging = 0;
int down = 0;
extern context_t screen;
extern int need_cursor;
void event_handler(event_t *event) {

	switch(event->type) {
	case EVENT_TYPE_MOUSEMOVE: {
		mousex += event->mousemove->x;
		mousey += event->mousemove->y;
		if(mousex < 0) mousex = 0;
		if(mousey < 0) mousey = 0;
		if(mousex > screen.w) mousex = screen.w;
		if(mousey > screen.h) mousey = screen.h;
		need_cursor = 1;
		int handle = get_window_handle(mousex, mousey);
		int win_x = 0, win_y = 0;
		window_t *win = NULL;
		if(handle) {
			win = GetWindowInfo(handle);
			if(!win) break;
			win_x = mousex-win->x;
			win_y = mousey-win->y;
			if(win->class & WC_MOUSEMOVE) {
				if(win->class & WC_NODECORATIONS) 
					PostEvent(win->tid, handle, EV_MMOVE,  win_x , win_y, 0, 0);
				else {
					if(win_x > 3 && win_x < win->w - 3 && win_y > 21 && win_y < win->h - 3)
					PostEvent(win->tid, handle, EV_MMOVE,  win_x - 3 , win_y - 21, 0, 0);
					}
			}
		}
		if (down)
		{
		    if (!dragging)
		    {
		if(win->class & WC_NODECORATIONS) break;
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
		
		if (dragging)
		{
		    if (curr_window)
		    {
			curr_window->x_drag += (mousex-last_x);
			curr_window->y_drag += (mousey-last_y);
			if(curr_window->x_drag < 0) {
				mousex -= curr_window->x_drag;
				curr_window->x_drag = 0;
			}
			if(curr_window->y_drag < 0) {
				mousey -= curr_window->y_drag;
				curr_window->y_drag = 0;
			}
			if(curr_window->x_drag + curr_window->w > screen.w) {
				mousex += screen.w - (curr_window->x_drag + curr_window->w);
				curr_window->x_drag = screen.w - curr_window->w;
			}
			if(curr_window->y_drag + curr_window->h > screen.h - 28) {
				mousey += screen.h - (curr_window->y_drag + curr_window->h + 28);
				curr_window->y_drag = screen.h - curr_window->h - 28;
			}
			last_x = mousex;
			last_y = mousey;
			DrawBorder(0);
		    }
		}

		}
		break;
	}
	case EVENT_TYPE_MOUSEDOWN: {
		down = 1;
		need_cursor = 1;
		int handle = get_window_handle(mousex, mousey);
		if(!handle) break;
		window_t *win = GetWindowInfo(handle);
		int win_x = mousex-win->x;
		int win_y = mousey-win->y;
		if(win->class & WC_NODECORATIONS) {
			PostEvent(win->tid, handle, EV_MDOWN,  win_x , win_y, 0, 0);
		} else {
			if(win_x > 3 && win_x < win->w - 3 && win_y > 21 && win_y < win->h - 3)
				PostEvent(win->tid, handle, EV_MDOWN,  win_x - 3 , win_y - 21, 0, 0);
		}
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
		if(win->class & WC_NODECORATIONS) {
			PostEvent(win->tid, handle, EV_MUP,  win_x , win_y, 0, 0);
		} else {
			if((win_x >= win->w - 21) && (win_y >= 5) && (win_x <= win->w - 5) && (win_y <= 19)) {
				PostEvent(win->tid, handle, EV_WINCLOSE, 0, 0, 0, 0);
				break;
			}
			if(win_x > 3 && win_x < win->w - 3 && win_y > 21 && win_y < win->h - 3)
				PostEvent(win->tid, handle, EV_MUP,  win_x - 3 , win_y - 21, 0, 0);
		}
		if(!win->active) {
			SetFocusTo(handle);
			need_cursor = 1;
		}
		break;
	}
}
