#include <fos/message.h>
#include <unistd.h>
#include "privatetypes.h"
extern fd_t __gui;
void RefreshWindow(int handle) {
	struct win_hndl *wh  = (struct win_hndl *) handle;
	struct message msg;
	msg.flags = 0;
	msg.send_size = 0;
	msg.recv_size = 0;
	msg.tid = __gui->thread;
	msg.arg[0] = WIN_CMD_REFRESHWINDOW;
	msg.arg[1] = wh->handle;
	send(&msg);
}
