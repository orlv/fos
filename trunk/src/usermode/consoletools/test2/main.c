#include <fos/fs.h>
#include <fos/message.h>
#include <fcntl.h>
#include <stdio.h>

tid_t gwinsy_tid;

typedef struct {
	unsigned int	ev_class;
	unsigned int	global_code;
	unsigned int	handle;
	unsigned int	a0;
	unsigned int	a1;
	unsigned int	a2;
	unsigned int	a3;
} event_t;

#define WIN_CMD_WAIT_EVENT (BASE_METHOD_N + 0)
#define WIN_CMD_SET_EVMASK (BASE_METHOD_N + 1)
#define WIN_CMD_GET_EVMASK (BASE_METHOD_N + 2)

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

int main(int argc, char **argv)
{
	int hndl = open("/dev/gwinsy/ipc", 0);
	if(hndl == -1) {
		printf("GWinSy not running\n");
	}
	printf("Connected to GWinSy\n");
	
	fd_t fd = (fd_t) hndl;
	gwinsy_tid = fd->thread;

	event_t ev;
	SetGlobalMask(EVG_MMOVE);
	printf("New mask: %x\n", GetGlobalMask());
	for(;;) {
		WaitEvent(&ev);
		printf("Got event %d\n", ev.ev_class);
		if(ev.ev_class == EV_GLOBAL) {
			printf(" It is a global event\n");
			if(ev.global_code == EVG_MMOVE) {
				printf(" Mouse moving\n");
				printf(	"  X: %d\n  Y: %d\n  DX: %d\n  DY: %d\n", ev.a0, ev.a1, ev.a2, ev.a3);
			}
		}
	}

	return 0;
}
