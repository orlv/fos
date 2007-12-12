#include <pgs/pgs.h>
#include <stdlib.h>
#include <string.h>
#include "privatetypes.h"
rootwindow_t *head = NULL;
int CreateControlsWindow(int x, int y, int w, int h, char *title,  int (*handler)(int, int, int, int, int, int), int style) {
	return (int) InternalCreateWindow(x, y, w, h, title, handler, style, 0, NULL);
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
