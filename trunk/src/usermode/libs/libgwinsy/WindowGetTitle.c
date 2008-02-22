#include <fos/message.h>
#include <fos/fs.h>
#include "privatetypes.h"

extern fd_t __gui;

int GetWindowTitle(int handle, char *buf) {
	struct message msg;
	msg.tid = __gui->thread;
	msg.recv_size = MAX_TITLE_LEN;
	msg.recv_buf = buf;
	msg.send_size = 0;
	msg.flags = 0;
	msg.arg[0] = WIN_CMD_GETTITLE;
	msg.arg[1] = handle;
	send(&msg);
	return msg.arg[0];
}
