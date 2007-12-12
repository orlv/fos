#include <fos/message.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "privatetypes.h"
extern fd_t __gui;
void DestroyWindow(int handle) {
	struct win_hndl *wh  = (struct win_hndl *) handle;
	struct message msg;
	msg.flags = 0;
	msg.send_size = 0;
	msg.recv_size = 0;
	msg.tid = __gui->thread;
	msg.arg[0] = WIN_CMD_DESTROYWINDOW;
	msg.arg[1] = wh->handle;
	send(&msg);
	free(wh);
	kmunmap((off_t) wh->data, ALIGN(wh->w * wh->h * wh->bpp, 4096));
}
