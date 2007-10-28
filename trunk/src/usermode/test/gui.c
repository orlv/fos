#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
 #include <unistd.h>

#include <fos/message.h>
#include <fos/fs.h>

#include <string.h>
#include "gui.h"
fd_t gui;

void GUIInit() {
	int iterations = 0;
	int gui_handle;
	do {
		iterations++;
		gui_handle = open("/dev/pgs", 0);
		sched_yield();
	} while(!gui_handle);
	//printf("Attached to PGS in %u iterations\n", iterations);
	gui = (fd_t )gui_handle; 
}
typedef struct {
	int parent;
	int x;
	int y;
	int w;
	int h;
	int class;
} create_win_t;
#define MAX_TITLE_LEN 64

int CreateWindow(int parent, int x, int y, int w, int h, char *caption, int flags) {
	char *buf = malloc(sizeof(create_win_t) + MAX_TITLE_LEN);
	create_win_t *struc = (create_win_t *) buf;
	char *title = buf + sizeof(create_win_t);
	strcpy(title, caption);
	struc->parent = parent;
	struc->x = x;
	struc->y = y;
	struc->w = w;
	struc->h = h;
	struc->class = flags;
	
	struct message msg;
	msg.recv_size = 0;
	msg.tid = gui->thread;
	msg.flags = 0;
	msg.a0 = WIN_CMD_CREATEWINDOW;
	msg.send_size = sizeof(create_win_t) + MAX_TITLE_LEN;
	msg.send_buf = buf;
	send(&msg);
	return msg.a0;
}
typedef struct {
	int class;
	int handle;
	int a0;
	int a1;
	int a2;
	int a3;
	void * reserved;
} event_t;
void WaitEvent(int *class, int *handle, int *a0, int *a1, int *a2, int *a3) {
	event_t event;
	struct message msg;
	msg.flags = 0;
	msg.a0 = WIN_CMD_WAIT_EVENT;
	msg.tid = gui->thread;
	msg.send_size = 0;
	msg.recv_size = sizeof(event_t);
	msg.recv_buf = &event;
	//printf("sent\n");
	send(&msg);
	//printf("received.\n");
	*class = event.class;
	*handle = event.handle;
	*a0 = event.a0;
	*a1 = event.a1;
	*a2 = event.a2;
	*a3 = event.a3;
}
void DestroyWindow(int handle) {
	struct message msg;
	msg.flags = 0;
	msg.send_size = 0;
	msg.recv_size = 0;
	msg.tid = gui->thread;
	msg.a0 = WIN_CMD_DESTROYWINDOW;
	msg.a1 = handle;
	send(&msg);
}
void GuiEnd() {
	struct message msg;
	msg.flags = 0;
	msg.send_size = 0;
	msg.recv_size = 0;
	msg.tid = gui->thread;
	msg.a0 = WIN_CMD_CLEANUP;
	send(&msg);
	gui = NULL;
}
