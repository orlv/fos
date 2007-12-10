#include <pgs/pgs.h>
#include <stdlib.h>
#include <string.h>
#include "privatetypes.h"
rootwindow_t *head = NULL;
int CreateControlsWindow(int w, int h, char *title,  int (*handler)(int, int, int, int, int, int)) {
	rootwindow_t *rw = malloc(sizeof(rootwindow_t));
	int hndl = CreateWindow(w, h, title, WC_WINDOW, &rw->evhandle);
	int *locatebuf = malloc(sizeof(int) * w * h);
	memset(locatebuf, 0, sizeof(int) * w * h);
	rw->handle = hndl;
	rw->locate = locatebuf;
	rw->w = w;
	rw->h = h;
	rw->control = NULL;
	rw->next = head;
	rw->handler = handler;
	head = rw;
	return (int) rw;
}

void DestroyControlsWindow(int hndl) {
	rootwindow_t *rw = (rootwindow_t *)hndl;
	DestroyWindow(rw->handle);
	for(control_t *ptr = rw->control; ptr; ptr=ptr->next)
		free(ptr);
	free(rw->locate);
	rootwindow_t *prev = NULL;
	for(rootwindow_t *ptr = head; ptr; prev = ptr, ptr = ptr->next) {
		if(ptr == rw && prev) {
			prev->next = rw->next;
			break;
		}
		if(ptr == rw && rw == head) {
			head = rw->next;
			break;
		}
	}
	free(rw);
}

void ControlsWindowVisible(int handle, int visible) {
	rootwindow_t *rw = (rootwindow_t *)handle;
	SetVisible(rw->handle, visible);
}
