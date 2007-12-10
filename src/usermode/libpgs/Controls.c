#include <pgs/pgs.h>
#include <stdlib.h>
#include <string.h>
#include "privatetypes.h"
void SetControlText(int handle, char *text) {
	control_t *c = (control_t *)handle;
	rootwindow_t *rw = c->win;
	free(c->text);
	c->text = malloc(strlen(text));
	strcpy(c->text, text);
	if(c->class == CONTROL_STATIC) {
		rect(rw->handle, c->x, c->y, c->w, c->h, 0xC3C3C3);
		pstring(rw->handle, c->x, c->y, 0x000000, c->text);
	} else {
		Draw3D(c->x, c->y, c->w, c->h, rw->handle, STYLE_BUTTON_NORMAL);
		pstring(rw->handle, c->x + (c->w - 8 * strlen(c->text)) / 2, c->y + (c->h - 16) / 2 , 0x000000, c->text);
	}
	RefreshWindow(rw->handle);
}
int CreateButton(int window, int x, int y, int w, int h, char *caption) {
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
	c->win=rw;
	rw->control = c;
	DrawLocateRect(rw->locate, x, y, w, h, (int) c, rw->w);
	Draw3D(x, y, w, h, rw->handle, STYLE_BUTTON_NORMAL);
	pstring(rw->handle,(w - 8 * len) / 2 + x, (h - 16) / 2 + y, 0x000000, caption);
	RefreshWindow(rw->handle);
	return (int)c;
}
int CreateStatic(int window, int x, int y, int w, int h, char *caption) {
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
	c->win=rw;
	rw->control = c;
	rect(rw->handle, x, y, w, h, 0xC3C3C3);
	pstring(rw->handle,x, y, 0x000000, caption);
	RefreshWindow(rw->handle);
	return (int)c;
}

void DestroyControl(int handle) {
	control_t *cntrl = (control_t *)handle;
	rootwindow_t *rw = cntrl->win;
	for(control_t *ptr = rw->control, *prev = NULL; ptr; prev = ptr, ptr=ptr->next) {
		if(ptr == cntrl && prev) {
			prev->next = ptr->next;
			break;
		}
		if(ptr == cntrl && ptr == rw->control) {
			rw->control = ptr->next;
			break;
		}
	}
	if(cntrl->class == CONTROL_BUTTON) 
		DrawLocateRect(rw->locate, cntrl->x, cntrl->y, cntrl->w, cntrl->h, 0, rw->w);
	free(cntrl->text);
	free(cntrl);
	rect(rw->handle, cntrl->x, cntrl->y, cntrl->w, cntrl->h, 0xc3c3c3);
	RefreshWindow(rw->handle);
}


