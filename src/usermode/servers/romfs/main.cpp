#include <stdio.h>
#include <fos/fos.h>
#include <fos/message.h>
#include <sys/stat.h>
#include <mutex.h>
#include <fcntl.h>
#include <string.h>
#include "romfs.h"

#define ROMFS_BUF_SIZE	0x2000

#define HANDLE_FILE	1
#define HANDLE_DIR	2

unsigned int last_handle = 0;
mutex_t q_locked = 0;

typedef struct h{
	struct h *next;
	char *data;
	romfs_inode_t in;
	unsigned int touched;
	unsigned int handle;
	int type;
	int tid;
} handle_t;

handle_t *head = NULL;

void outdated();

handle_t *resolve_handle(unsigned int hndl, int type) {
	while(!mutex_try_lock(&q_locked))
		sched_yield();
	for(handle_t *p = head; p; p = p->next) {
		if(p->handle == hndl && type == p->type) {
			mutex_unlock(&q_locked);
			return p;
		}
	}
	printf("warning: bogus handle %u\n", hndl);
	mutex_unlock(&q_locked);
	return NULL;
}

asmlinkage int main(int argc, char *argv[]) {
	romfs *rfs = new romfs("/mnt/modules/initrd.gz");

	char *buffer = new char[ROMFS_BUF_SIZE];
	
	struct stat *statbuf = new struct stat;
	//size_t size;

	struct message msg;
	struct dirent dent;
	thread_create((off_t) outdated, 0);
	resmgr_attach("/");
	while(1) {
		msg.tid = 0;
		msg.recv_buf = buffer;
		msg.recv_size = ROMFS_BUF_SIZE;
		msg.flags = 0;
		receive(&msg);
		switch(msg.arg[0]) {
		case FS_CMD_ACCESS: {
			handle_t *hndl = new handle_t;
			buffer[msg.recv_size] = 0;
			char *data = rfs->search_path(buffer, &hndl->in, NEED_FILE);
			if(!data) {
				msg.arg[0] = 0;
				msg.arg[1] = ROMFS_BUF_SIZE;
				msg.arg[2] = ERR_NO_SUCH_FILE;
				msg.arg[3] = 0;
				msg.send_size = 0;
				delete hndl;
				break;
			} else {
				while(!mutex_try_lock(&q_locked)) {
	 				 sched_yield(); }
     				last_handle ++;
				hndl->data = data;
				hndl->touched = uptime();
				hndl->handle = last_handle;
				hndl->next = head;
				hndl->type = HANDLE_FILE;
				hndl->tid = msg.tid;
				head = hndl;
				mutex_unlock(&q_locked);
				msg.arg[0] = last_handle;
				msg.arg[1] = ROMFS_BUF_SIZE;
				msg.arg[2] = NO_ERR;
				msg.arg[3] = hndl->in.size;
				msg.send_size = 0;
				break;
			}
		}
		case FS_CMD_STAT: {
			romfs_inode_t in;
			buffer[msg.recv_size] = 0;
			if(!rfs->search_path(buffer, &in, NEED_DIR_OR_FILE)) {
				msg.send_size = 0;
				msg.arg[2] =ERR_NO_SUCH_FILE;
				break;
			}

			rfs->stat(&in, statbuf);
			msg.arg[1] = ROMFS_BUF_SIZE;
			msg.arg[2] = NO_ERR;
			msg.send_size = sizeof(struct stat);
			msg.send_buf = statbuf;
			break;
		}
		case FS_CMD_FSTAT: {
			handle_t *h = resolve_handle(msg.arg[1], HANDLE_FILE);
			if(!h) {
				msg.send_size = 0;
				msg.arg[2] = ERR_NO_SUCH_FILE;
				break;
			}
			h->touched = uptime();		
			rfs->stat(&h->in, statbuf);
			msg.arg[1] = ROMFS_BUF_SIZE;
			msg.arg[2] = NO_ERR;
			msg.send_size = sizeof(struct stat);
			msg.send_buf = statbuf;
			break;
		}
		case FS_CMD_READ: {
			size_t size = msg.send_size;
			if(size > ROMFS_BUF_SIZE)
				size = ROMFS_BUF_SIZE;
			handle_t *h = resolve_handle(msg.arg[1], HANDLE_FILE);
			if(!h) {
				msg.arg[2] = ERR_NO_SUCH_FILE;
				msg.arg[1] = 0;
				msg.send_size = 0;
				break;
			}
			h->touched = uptime();
			unsigned int readed = rfs->read(&h->in, h->data, buffer,  size, msg.arg[2]);
			if(readed < size) 
				msg.arg[2] = ERR_EOF;
			else
				msg.arg[2] = NO_ERR;
			msg.arg[0] = readed;
			msg.arg[1] = h->in.size;
			msg.send_size = msg.arg[0];
			msg.send_buf = buffer;
			break;
		}
		case FS_CMD_DIRCLOSE:
		case FS_CMD_CLOSE:
			while(!mutex_try_lock(&q_locked))
				sched_yield();
			for(handle_t *p = head, *prev = NULL; p; prev = p, p = p->next) {
				if(p->handle == msg.arg[1]) {
					if(prev) 
						prev->next = p->next;
					else
						head = p->next;
					delete p;
					break;
				}
			}
			mutex_unlock(&q_locked);
			break;
		case FS_CMD_DIROPEN: {
			handle_t *hndl = new handle_t;
			buffer[msg.recv_size] = 0;
			char *data = rfs->search_path(buffer, &hndl->in, NEED_DIR);
			if(!data) {
				msg.arg[0] = 0;
				msg.arg[1] = 0;
				msg.arg[2] = ERR_NO_SUCH_FILE;
				msg.send_size = 0;
				delete hndl;
				break;
			} else {
				while(!mutex_try_lock(&q_locked))
	 				 sched_yield();
				last_handle ++;
				hndl->data = data;
				memcpy(&hndl->in, data, sizeof(hndl->in));
				hndl->touched = uptime();
				hndl->handle = last_handle;
				hndl->next = head;
				hndl->type = HANDLE_DIR;
				hndl->tid = msg.tid;
				head = hndl;
				mutex_unlock(&q_locked);
				msg.arg[0] = last_handle;
				msg.arg[1] = rfs->get_ent_count(&hndl->in);
				msg.arg[2] = NO_ERR;
				msg.send_size = 0;
				break;
			}
		}
		case FS_CMD_DIRREAD: {
			handle_t *h = resolve_handle(msg.arg[1], HANDLE_DIR);
			if(!h) {
				msg.arg[2] = ERR_NO_SUCH_FILE;
				msg.send_size = 0;
				break;
			}
			romfs_inode_t *in = rfs->get_inode(&h->in, msg.arg[2]);
			if(!in) {
				msg.arg[2] = ERR_EOF;
				msg.send_size = 0;
				break;
			}
			dent.d_ino = in->checksum;
			dent.d_reclen = strlen(in->name);
			if(msg.arg[2] > 0)
				strcpy(dent.d_name, in->name);
			else
				strcpy(dent.d_name, ".");
			msg.send_size = sizeof(struct dirent);
			msg.send_buf = &dent;
			msg.arg[2] = NO_ERR;
			break;			
		}
		case FS_CMD_POSIX_ACCESS: {
			romfs_inode_t in;
			buffer[msg.recv_size] = 0;
			if(!rfs->search_path(buffer, &in, NEED_DIR_OR_FILE)) {
				msg.arg[2] = ERR_NO_SUCH_FILE;
				msg.send_size = 0;
				break;
			}
			if(msg.arg[1] & W_OK) {
				msg.arg[2] = ERR_ACCESS_DENIED;
				msg.send_size = 0;
				break;
			}
			msg.arg[0] = NO_ERR;
			msg.send_size = 0;
			break;
		}
		default:
			printf("romfs: unknown command %u %u %u %u\n", msg.arg[0], msg.arg[1], msg.arg[2], msg.arg[3]);
			msg.arg[0] = 0;
			msg.arg[2] = ERR_UNKNOWN_METHOD;
			msg.send_size = 0;
		}
		reply(&msg);
	}
	return 0;
	
}

void outdated() {
	while(1) {
		struct message msg;
		msg.tid = 0;
		msg.recv_buf = NULL;
		msg.recv_size = 0;
		msg.flags = MSG_ASYNC;
		alarm(20000);	// это в районе минуты на самом деле.
		receive(&msg);
		reply(&msg);
 		while(!mutex_try_lock(&q_locked))
	 		sched_yield();
		for(handle_t *ptr = head, *prev = NULL; ptr; prev = ptr, ptr = ptr->next) {
			if(ptr->touched < uptime() - 20000) {
				printf("romfs warning: handle %u (by %u) is too old.\n", ptr->handle, ptr->tid);
				if(prev)
					prev->next = ptr->next;
				else
					head = ptr->next;
				delete ptr;
			}
		}
		mutex_unlock(&q_locked);
	}
}


