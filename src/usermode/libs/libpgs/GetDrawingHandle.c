#include "privatetypes.h"
int GetDrawingHandle(int handle) {
	rootwindow_t *rw = (rootwindow_t *)handle;
	return rw->handle;
}
