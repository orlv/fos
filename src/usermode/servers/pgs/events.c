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
//		RedrawCursor();
		if (down)
		{
		    if (!dragging)
		    {
			int handle = get_window_handle(mousex, mousey);
			if (handle)
			{
			    window_t *win = GetWindowInfo(handle);
			    int win_x = mousex-win->x;
			    int win_y = mousey-win->y;

			    if ((win_x>=3) && (win_y>=3) && (win_y<=18) && (win_y<=(win->w-6)))
			    {
				curr_window = win;
				last_x = mousex;
				last_y = mousey;
				SetFocusTo(handle);
			    }
			}
			dragging = 1;
		    }
			need_refresh = 1;
		}
		else
		{
		    dragging = 0;
		    curr_window = 0;
		}
		
		if (dragging)
		{
		    if (curr_window)
		    {
			curr_window->x += (mousex-last_x);
			curr_window->y += (mousey-last_y);
			last_x = mousex;
			last_y = mousey;
		    }
		}
		need_cursor = 1;
		break;
	case EVENT_TYPE_MOUSEDOWN:
		down = 1;
		need_cursor = 1;
		break;
	case EVENT_TYPE_MOUSEUP:
		down = 0;
		int handle = get_window_handle(mousex, mousey);
		if(handle != 0) 
			SetFocusTo(handle);
		need_cursor = 1;
		break;
	}
}
