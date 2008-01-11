#include <fgs/fgs.h>
#include <string.h>
#include "privatetypes.h"
// ацццццкая функция
void ShiftWindowUp(int handle, int count) {
	struct win_hndl *wh = (struct win_hndl *)handle;
	int y = wh->margin_up;
	int pitch = (wh->w + wh->margin_left + wh->margin_right);
	int off = (count + 0) * pitch * 2;
	memcpy(wh->data + y * pitch, wh->data + y * pitch + off, (pitch * wh->h - count) * 2);
	rect(handle, 0, wh->h - count, wh->w, count, 0);
}
