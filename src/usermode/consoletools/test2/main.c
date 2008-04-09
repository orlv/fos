#include <fos/fs.h>
#include <fos/message.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

tid_t gwinsy_tid = 0, map_tid = 0;

typedef struct {
	unsigned int	ev_class;
	unsigned int	global_code;
	unsigned int	handle;
	unsigned int	a0;
	unsigned int	a1;
	unsigned int	a2;
	unsigned int	a3;
} event_t;

typedef struct {
	int	x;
	int	y;
	unsigned int w;
	unsigned int h;
	unsigned int parent;
	char title[64];
} create_win_t;

typedef struct {
	unsigned int parent;
	int x;
	int y;
	unsigned int w;
	unsigned int h;
	int visible;
	char title[64];
} win_attr_t;

#define WIN_CMD_WAIT_EVENT (BASE_METHOD_N + 0)
#define WIN_CMD_SET_EVMASK (BASE_METHOD_N + 1)
#define WIN_CMD_GET_EVMASK (BASE_METHOD_N + 2)
#define WIN_CMD_CREATE_WINDOW 	(BASE_METHOD_N + 4)
#define WIN_CMD_GET_WINDOW_ATTR	(BASE_METHOD_N + 5)
#define WIN_CMD_SET_WINDOW_ATTR	(BASE_METHOD_N + 6)

#define WIN_CMD_MAP_WINDOW 	(BASE_METHOD_N + 0)

#define EV_GLOBAL	1
#define EVG_MMOVE	(1 << 0)

void SetGlobalMask(unsigned int  mask) {
	struct message msg;
	msg.tid = gwinsy_tid;
	msg.flags = 0;
	msg.send_size = 0;
	msg.recv_size = 0;
	msg.arg[0] = WIN_CMD_SET_EVMASK;
	msg.arg[1] = mask;
	send(&msg);
}

unsigned int GetGlobalMask() {
	struct message msg;
	msg.tid = gwinsy_tid;
	msg.flags = 0;
	msg.send_size = 0;
	msg.recv_size = 0;
	msg.arg[0] = WIN_CMD_GET_EVMASK;
	send(&msg);
	return msg.arg[0];
}

void WaitEvent(event_t *buf) {
	struct message msg;
	msg.tid = gwinsy_tid;
	msg.flags = 0;
	msg.send_size = 0;
	msg.recv_buf = buf;
	msg.recv_size = sizeof(event_t);
	msg.arg[0] = WIN_CMD_WAIT_EVENT;
	send(&msg);
}

unsigned int CreateWindow(int x, int y, unsigned int w, unsigned int h, const char *title, unsigned int parent) {
	create_win_t creat;
	creat.x = x;
	creat.y = y;
	creat.w = w;
	creat.h = h;
	creat.parent = parent;
	strncpy(creat.title, title, 64);
	struct message msg;
	msg.tid = gwinsy_tid;
	msg.flags = 0;
	msg.send_size = sizeof(create_win_t);
	msg.send_buf = &creat;
	msg.recv_size = 0;
	msg.arg[0] = WIN_CMD_CREATE_WINDOW;
	send(&msg);

	unsigned int hndl = msg.arg[0];
	unsigned long buf_sz = msg.arg[1];

	if(hndl == 0)
		return 0;

	void *buf = NULL;
	buf = kmmap(0, buf_sz, 0, 0);
	if(buf == NULL)
		return 0;


	msg.tid = map_tid;
	msg.flags = MSG_MEM_SHARE;
	msg.send_size = buf_sz;
	msg.recv_size = 0;
	msg.send_buf = buf;
	msg.arg[0] = WIN_CMD_MAP_WINDOW;
	msg.arg[1] = hndl;
	send(&msg);

	if(msg.arg[0] == -1)
		return 0;

	return hndl;
}

int GetWindowAttr(unsigned int hndl, win_attr_t *attr) {
	struct message msg;
	msg.tid = gwinsy_tid;
	msg.flags = 0;
	msg.send_size = 0;
	msg.recv_size = sizeof(win_attr_t);
	msg.recv_buf = attr;
	msg.arg[0] = WIN_CMD_GET_WINDOW_ATTR;
	msg.arg[1] = hndl;
	send(&msg);
	return msg.arg[1];
}

int SetWindowAttr(unsigned int hndl, win_attr_t *attr) {
	struct message msg;
	msg.tid = gwinsy_tid;
	msg.flags = 0;
	msg.send_size = sizeof(win_attr_t);
	msg.send_buf = attr;
	msg.recv_size = 0;
	msg.arg[0] = WIN_CMD_SET_WINDOW_ATTR;
	msg.arg[1] = hndl;
	send(&msg);
	return msg.arg[1];
}

int main(int argc, char **argv)
{
	int hndl = open("/dev/gwinsy/ipc", 0);
	if(hndl == -1) {
		printf("GWinSy not running\n");
		return 1;
	}

	fd_t fd = (fd_t) hndl;
	gwinsy_tid = fd->thread;

	hndl = open("/dev/gwinsy/map", 0);
	if(hndl == -1) {
		printf("GWinSy not running\n");
		return 1;
	}
	printf("Connected to GWinSy\n");
	
	fd = (fd_t) hndl;
	map_tid = fd->thread;


	event_t ev;
	SetGlobalMask(0);

	unsigned int new_win = CreateWindow(100, 100, 100, 100, "Hello, World!", 0);

	printf("Created new window: %d\n", new_win);

	win_attr_t attr;
	GetWindowAttr(new_win, &attr);

	attr.visible = 1;

	SetWindowAttr(new_win, &attr);

	for(;;) {
		WaitEvent(&ev);
		printf("Got event %d\n", ev.ev_class);
	}

	return 0;
}
