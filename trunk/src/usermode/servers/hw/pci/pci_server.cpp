/*
	Copyright (C) 2007 Michael Zhilin
	FOS system
	PCI Configuration Server
 */

#define RESOURCE_NAME_TEMP(a) "/dev/" #a
#define RECV_BUF_SIZE 2048

#define SERVER_NAME pci_server
#define RESOURCE_NAME RESOURCE_NAME_TEMP(pci_server) 

/**
 * Global headers
 */
#include <fos/fos.h>
#include <fos/fs.h>
#include <fos/message.h>
#include <types.h>
#include <stdio.h>
#include <errno.h>

/**
 * Local headers
 */

#include "pci_def.h"
#include "pci_types.h"
#include "pci_const.h"
#include "pci.h"

asmlinkage int main()
{ 
	message msg;

	const char *resource_name = RESOURCE_NAME;
	char *buffer = new char[RECV_BUF_SIZE];

	alarm(2000);
    msg.recv_size = 0;
    msg.tid = _MSG_SENDER_SIGNAL;
    receive(&msg);

	printf("Resource allocation: %s \n", resource_name);

	if(resmgr_attach(resource_name) != RES_SUCCESS){
		printf("Resource allocation error: %s \n", resource_name);
		return -1;
	}
	
	get_pci_access();

	printf("Success \n");

	while (1) {
		msg.tid = _MSG_SENDER_ANY;
		msg.recv_size = RECV_BUF_SIZE;
		msg.recv_buf = buffer;
		receive(&msg);

		switch(msg.arg[0]){
			case FS_CMD_ACCESS:
				printf("Attempt of access... \n");
				msg.arg[0] = 1;
				msg.arg[1] = RECV_BUF_SIZE;
				msg.arg[2] = NO_ERR;
			break;
	
			case FS_CMD_WRITE:
				printf("Attempt of writing... \n");
			break;
		
			default:
				printf("Unknown \n");
				msg.arg[0] = 0;
				msg.arg[2] = ERR_UNKNOWN_CMD;
		}
	
		msg.send_size = 0;
		reply(&msg);
	}
	return 0;
} 
