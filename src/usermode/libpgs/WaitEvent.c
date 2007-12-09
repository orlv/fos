#include <fos/message.h>
#include <unistd.h>
#include "privatetypes.h"
extern fd_t __gui;
void WaitEvent(int *class, int *handle, int *a0, int *a1, int *a2, int *a3) {
	event_t event;
	struct message msg;
	msg.flags = 0;
	msg.arg[0] = WIN_CMD_WAIT_EVENT;
	msg.tid = __gui->thread;
	msg.send_size = 0;
	msg.recv_size = sizeof(event_t);
	msg.recv_buf = &event;
	send(&msg);
	*class = event.class;
	*handle = event.handle;
	*a0 = event.a0;
	*a1 = event.a1;
	*a2 = event.a2;
	*a3 = event.a3;
}

