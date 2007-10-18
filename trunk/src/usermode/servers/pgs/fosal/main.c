#include <fos/message.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

#include <gui/types.h>

unsigned short *lfb;
mode_definition_t __current_mode;

mode_definition_t graphics_init() {
	printf("FOS layer starting up\n");
	int tty;
	do {
	tty = open("/dev/tty", 0);
	} while(!tty);
	if(!tty) {
		printf("FOSAL: can't connect to video server\n");
		exit(1);
	}

	struct fd *fd = (struct fd *) tty;
	struct message msg;
	msg.a0 = 0xfff0;
	msg.send_size = 0;
	msg.recv_size = 0;
	msg.flags = 0;
	msg.tid = fd->thread;
	send((struct message *)&msg);
  	if(msg.a2 != NO_ERR) {
		printf("FOSAL: server error. Is server supports extended functions 0xfffe / 0xffff?\n");
		exit(1);
	}
 	printf("Video mode %ux%u, lfb @ 0x%x\n", msg.a1, msg.a3, msg.a0);
	lfb = (unsigned short *) kmmap(0, msg.a1 * msg.a3 * 2, 0, msg.a0);
	if(!lfb) {
		printf("FOSAL: failed mapping LFB\n");
		exit(1);
	}
	printf("LFB mapped to 0x%x\n", lfb);
	__current_mode.width= msg.a1;
	__current_mode.height = msg.a3;
	__current_mode.bpp = 2;
	return __current_mode;
} 
