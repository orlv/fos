#include <types.h>
#include <fos/fos.h>
#include <fos/message.h>
#include <sys/stat.h>
#include <stdlib.h>
#define ROMFS_BUF_SIZE	0x2000

#include "romfs.h"
int main(int argc, char *argv[]) {
	if(romfs_init())
		return 1;
	resmgr_attach("/initrd");

	char *buffer = malloc(ROMFS_BUF_SIZE);
	
	struct stat *statbuf = malloc(sizeof(struct stat));
	size_t size;

	struct message msg;


	while(1) {
		msg.tid = _MSG_SENDER_ANY;
		msg.recv_buf = buffer;
		msg.recv_size = ROMFS_BUF_SIZE;
		msg.flags = 0;
		receive(&msg);

		switch(msg.arg[0]) {
		case FS_CMD_ACCESS:
			buffer[msg.recv_size] = 0;
			printf("romfs: access to [%s]\n", buffer);
			msg.arg[0] = 0;
			msg.arg[1] = ROMFS_BUF_SIZE;
			msg.arg[2] = NO_ERR;
			msg.send_size = 0;
			break;
		default:
			msg.arg[0] = 0;
			msg.arg[2] = ERR_UNKNOWN_CMD;
			msg.send_size = 0;
		}
		reply(&msg);
	}
	return 0;
}
