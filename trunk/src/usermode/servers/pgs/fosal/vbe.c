#include <fos/message.h>
#include <fos/fs.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "vbe.h"
#define VBESRV_CMD_SET_MODE (BASE_CMD_N + 0)

struct vbe_mode_info_block *vbe;
void vbe_set_mode(u16_t mode) {
	vbe = malloc( sizeof(struct vbe_mode_info_block));
	int fd;
	struct message msg;
	do {
  		fd = open("/dev/vbe", 0);
	} while(fd == -1);
	msg.a0 = VBESRV_CMD_SET_MODE;
	msg.a1 = mode;
	msg.send_size = 0;
	msg.recv_size = sizeof(struct vbe_mode_info_block);
	msg.recv_buf = vbe;
	msg.tid = ((fd_t)fd)->thread;
	msg.flags = 0;
	send((struct message *)&msg);
	close(fd);
	if(!msg.a0) {
		printf("Setting mode failed. :(\n");
		exit(1);
	}
	
}
