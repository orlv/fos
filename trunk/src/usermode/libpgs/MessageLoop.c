#include <stdio.h>
#include <string.h>
#include <pgs/pgs.h>
#include <pgs/controls.h>
#include "privatetypes.h"
void ControlsMessageLoop() {
	int class, handle, a0, a1, a2, a3;
	rootwindow_t *win;
	control_t *control;
	while(1) {
		WaitEvent(&class, &handle, &a0, &a1, &a2, &a3);
		win = ResolveEventHandle(handle);
		if(!win) {
			printf("Warning: can't handle event for window %u\n", handle);
			continue;
		}
		switch(class) {
		case EV_WINCLOSE:
			if(!(win->handler)((int) win, class, a0, a1, a2, a3)) {
				printf("Window destroying!\n");
				return;
			}
			break;
		case EV_MDOWN:
			control = ResolveMouseCoord(win, a0, a1);
			if(!control) 
				break;
			if(control->class == CONTROL_BUTTON) {
				Draw3D(control->x, control->y, control->w, control->h, win->handle, STYLE_BUTTON_DOWN);
				pstring(win->handle,(control->w - 8 * strlen(control->text)) / 2 + control->x, (control->h - 16) / 2 + control->y, 0x000000, control->text);
				RefreshWindow(win->handle);
				control->down = 1;
			}
			break;
		case EV_MUP:
			if(win->menu_of) {
				(win->handler)((int) win, EVC_CLICK, a0, a1 , 0, 0);
				 break;
			}
			for(control_t *ptr = win->control; ptr; ptr=ptr->next) {
				if(ptr->down) {
					ptr->down = 0;
					if(ptr->class == CONTROL_BUTTON) {
						Draw3D(ptr->x, ptr->y, ptr->w, ptr->h, win->handle, STYLE_BUTTON_NORMAL);
						pstring(win->handle,(ptr->w - 8 * strlen(ptr->text)) / 2 + ptr->x, (ptr->h - 16) / 2 + ptr->y, 0x000000, ptr->text);
						RefreshWindow(win->handle);
					}
				}
			}
			control = ResolveMouseCoord(win, a0, a1);
			if(!control)
				break;
			if(control->class == CONTROL_BUTTON) {
				if(!(win->handler)((int) control, EVC_CLICK, a0 - control->x, a1 - control->y, 0, 0)) break;
			}
			break;
		case EV_MMOVE:
			if(win->menu_of) {
				if(a0 < 4 || a1 < 4 || a0 > win->w - 4 || a1 > win->h - 4) {
					if(win->menu->selected) {
						rect(win->handle, 4, 4 + (win->menu->selected - 1) * 20, win->w - 8, 20, 0xC3C3C3);
						pstring(win->handle, 4, 6 + (win->menu->selected - 1) * 20, 0, win->menu->items[win->menu->selected - 1]);
						RefreshWindow(win->handle);
						win->menu->selected = 0;
					}
					break;
				}
				int item = a1 / 20;
				if(item + 1 > win->menu->count) break;
				rect(win->handle, 4, 4 + item * 20, win->w - 8, 20, 0x78);
				pstring(win->handle, 4, 6 + item * 20, 0xFFFFFF, win->menu->items[item]);
				if(win->menu->selected != item + 1 && win->menu->selected) {
					rect(win->handle, 4, 4 + (win->menu->selected - 1) * 20, win->w - 8, 20, 0xC3C3C3);
					pstring(win->handle, 4, 6 + (win->menu->selected - 1) * 20, 0, win->menu->items[win->menu->selected - 1]);
				}
				win->menu->selected = item + 1;
				RefreshWindow(win->handle);
			}
			break;
		default:
			printf("Event %u(%u, %u, %u, %u) for window at ptr 0x%x\n", class, a0, a1, a2, a3, win);
		}
	}
}

