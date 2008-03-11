/*
 * Copyright (c) 2008 Sergey Gridassov
 * Original code (PCI Configuration Server) (c) 2007  Michael Zhilin
 */

#include <stdio.h>
#include <fos/message.h>
#include <fos/fos.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "pci.h"

unsigned int last_handle = 0;
mutex_t q_locked = 0;

typedef struct h{
	struct h *next;
	u32_t addr;
	unsigned int touched;
	unsigned int handle;
	int tid;
} handle_t;

static handle_t *head = NULL;

size_t pci_write(u32_t addr, void *buffer, u32_t size, int reg) {
	if(reg > PCI_REG_ID_MAX)
		return 0;
	switch(size) {
	case 1:
		poutb(addr, reg, *((u8_t *)buffer));
		return 1;
	case 2:
		poutw(addr, reg, *((u16_t *)buffer));
		return 2;
	case 4:
		poutl(addr, reg, *((u32_t *)buffer));
		return 4;
	default:
		return 0;
	}
		
}

size_t pci_read(u32_t addr, void *buffer, u32_t size, int reg) {
	if(reg > PCI_REG_ID_MAX)
		return 0;
	switch(size) {
	case 1:
		*((u8_t *)buffer) = pinb(addr, reg);
		return 1;
	case 2:
		*((u16_t *)buffer) = pinw(addr, reg);
		return 2;
	case 4:
		*((u32_t *)buffer) = pinl(addr, reg);
		return 4;
	default:
		return 0;
	}
		
}

handle_t *resolve_handle(unsigned int hndl) {
	while(!mutex_try_lock(&q_locked))
		sched_yield();
	for(handle_t *p = head; p; p = p->next) {
		if(p->handle == hndl) {
			mutex_unlock(&q_locked);
			return p;
		}
	}
	printf("warning: bogus handle %u\n", hndl);
	mutex_unlock(&q_locked);
	return NULL;
}

int parse_address(char *string, u32_t *addr) {
	unsigned int parts[4];
	char *ptr = string;
	for(int i = 0; i < 4; i++) {
		ptr = strchr(ptr, '/');
		if(ptr == NULL)
			return -1;
		ptr++;
		parts[i] = atoi(ptr);
	}
	if(	((parts[3] & 1) != parts[3])	||	/* enable только 1 или 0 */
		(parts[2] > PCI_FUNC_ID_MAX)	||	/* function от 0 до 7 */
		(parts[1] > PCI_SLOT_ID_MAX)	||	/* slot от 0 до 31 */
		(parts[0] > PCI_BUS_ID_MAX))		/* bus от 0 до 255 */
		return -1;
	

	*addr =		(parts[3] << 31)	|
			(parts[0] << 16)	|
			(parts[1] << 11)	|
			(parts[2] << 8);

	return 0;
}

void outdated() {
	while(1) {
		struct message msg;
		msg.tid = 0;
		msg.recv_buf = NULL;
		msg.recv_size = 0;
		msg.flags = 0;
		alarm(30000);	// здесь я поменьше поставил - 30 сек - вполне хватит
		receive(&msg);
		reply(&msg);
		while(!mutex_try_lock(&q_locked))
	 		sched_yield();
		for(handle_t *ptr = head, *prev = NULL; ptr; prev = ptr, ptr = ptr->next) {
			if(ptr->touched < uptime() - 30000) {
				printf("pci warning: handle %u (by %u) is too old.\n", ptr->handle, ptr->tid);
				if(prev)
					prev->next = ptr->next;
				else
					head = ptr->next;
				free(ptr);
			}
		}
		mutex_unlock(&q_locked);
	}
}

int main(int argc, char *argv[]) {
	resmgr_attach("/dev/pci");
	char *buffer = malloc(256);
	struct message msg;
	thread_create((off_t) outdated);
	while(1) {
		msg.tid = 0;
		msg.recv_buf = buffer;
		msg.recv_size = 256;
		msg.flags = 0;
		receive(&msg);
		switch(msg.arg[0]) {
		case FS_CMD_ACCESS: {
			buffer[msg.recv_size] = 0;
			u32_t addr;
			if(parse_address(buffer, &addr) < 0) {
				msg.arg[0] = 0;
				msg.arg[1] = 256;
				msg.arg[2] = ERR_NO_SUCH_FILE;
				msg.arg[3] = 0;
				msg.send_size = 0;
				break;
			} else {
				while(!mutex_try_lock(&q_locked))
	 				 sched_yield();
				last_handle ++;
				handle_t *hndl = malloc(sizeof(handle_t));
				hndl->addr = addr;
				hndl->touched = uptime();
				hndl->handle = last_handle;
				hndl->next = head;
				hndl->tid = msg.tid;
				head = hndl;
				mutex_unlock(&q_locked);
				msg.arg[0] = last_handle;
				msg.arg[1] = 256;
				msg.arg[2] = NO_ERR;
				msg.arg[3] = 255;
				msg.send_size = 0;
				break;
			}
		}
		case FS_CMD_WRITE: {
			size_t size = msg.recv_size;
			if(size > 256)
				size = 256;
			handle_t *h = resolve_handle(msg.arg[1]);
			if(!h) {
				msg.arg[2] = ERR_NO_SUCH_FILE;
				msg.arg[1] = 0;
				msg.send_size = 0;
				break;
			}
			h->touched = uptime();
			msg.arg[0] = pci_write(h->addr, buffer,  size, msg.arg[2]);
			msg.arg[1] = 255;
			msg.arg[2] = msg.arg[0] ? NO_ERR : ERR_EOF;

			msg.send_size = 0;
			break;
		}
		case FS_CMD_READ: {
			size_t size = msg.send_size;
			if(size > 256)
				size = 256;
			handle_t *h = resolve_handle(msg.arg[1]);
			if(!h) {
				msg.arg[2] = ERR_NO_SUCH_FILE;
				msg.arg[1] = 0;
				msg.send_size = 0;
				break;
			}
			h->touched = uptime();
			msg.arg[0] = pci_read(h->addr, buffer,  size, msg.arg[2]);
			msg.arg[1] = 255;
			msg.arg[2] = msg.arg[0] ? NO_ERR : ERR_EOF;
			msg.send_size = msg.arg[0];
			msg.send_buf = buffer;
			break;
		}
		case FS_CMD_CLOSE:
			while(!mutex_try_lock(&q_locked))
				sched_yield();
			for(handle_t *p = head, *prev = NULL; p; prev = p, p = p->next) {
				if(p->handle == msg.arg[1]) {
					if(prev) 
						prev->next = p->next;
					else
						head = p->next;
					free(p);
					break;
				}
			}
			mutex_unlock(&q_locked);
			break;
		default:
			printf("pcid: unknown command %u %u %u %u\n", msg.arg[0], msg.arg[1], msg.arg[2], msg.arg[3]);
			msg.arg[0] = 0;
			msg.arg[2] = ERR_UNKNOWN_CMD;
			msg.send_size = 0;
		}
		reply(&msg);
	}
	return 0;
}

