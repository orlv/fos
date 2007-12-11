#include <fos/message.h>
#include <unistd.h>
#include <stdlib.h>
#include "privatetypes.h"
extern fd_t __gui, __gui_canvas;
extern char *__fnt;
void GuiEnd() {
	struct message msg;
	msg.flags = 0;
	msg.send_size = 0;
	msg.recv_size = 0;
	msg.tid = __gui->thread;
	msg.arg[0] = WIN_CMD_CLEANUP;
	printf("sending ");
	send(&msg);
	printf("ok\n");
	close((int) __gui);
	close((int) __gui_canvas);
	__gui = NULL;
	__gui_canvas = NULL;
	if(__fnt)
		free(__fnt);
}
