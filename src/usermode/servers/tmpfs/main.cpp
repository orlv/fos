#include <stdio.h>
#include <fos/fos.h>
#include <fos/message.h>
#include <sys/stat.h>
#include <mutex.h>
#include <string.h>
#include <fcntl.h>
#include "tmpfs.h"
 
#define TMPFS_BUF_SIZE	0x2000
mutex_t q_locked = 0;
#define HANDLE_FILE	1
#define HANDLE_DIR	2
unsigned int last_handle = 0;
typedef struct h {
	struct h *next;
	unsigned int touched;
	unsigned int handle;
	int type;
	tmpfs_file_t *target;
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
	tmpfs *tfs = new tmpfs();
	resmgr_attach("/tmp");

	char *buffer = new char[TMPFS_BUF_SIZE];
	
	struct stat *statbuf = new struct stat;
	//size_t size;

	struct message msg;
	struct dirent dent;
	thread_create((off_t) outdated, 0);
	while(1) {
	  msg.tid = 0;
		msg.recv_buf = buffer;
		msg.recv_size = TMPFS_BUF_SIZE;
		msg.flags = 0;
		receive(&msg);
		printf("tmpfs: activity %d\n", msg.arg[0]);
		switch(msg.arg[0]) {
		case FS_CMD_ACCESS: {
			handle_t *hndl = new handle_t;
			buffer[msg.recv_size] = 0;
			int exists = (tfs->locate_file(buffer, &hndl->target) == 0);
			printf("opening %s with mode %d\n", buffer, msg.arg[1]);
			if(msg.arg[1] & O_EXCL && exists) {
				msg.arg[0] = 0;
				msg.arg[1] = TMPFS_BUF_SIZE;
				msg.arg[2] = ERR_NO_SUCH_FILE;
				msg.arg[3] = 0;
				msg.send_size = 0;
				delete hndl;
				break;			
			}
			if((msg.arg[1] & O_CREAT && !exists) || msg.arg[1] & O_TRUNC) 
				tfs->create_file(buffer);
		
			if(!(msg.arg[1] & O_CREAT) && !exists) {
				printf("%s not exist\n", buffer);
				msg.arg[0] = 0;
				msg.arg[1] = TMPFS_BUF_SIZE;
				msg.arg[2] = ERR_NO_SUCH_FILE;
				msg.arg[3] = 0;
				msg.send_size = 0;
				delete hndl;
				break;
			} else {
				if(!exists) tfs->locate_file(buffer, &hndl->target);
				while(!mutex_try_lock(&q_locked))
	 				 sched_yield();

				last_handle ++;
				hndl->touched = uptime();
				hndl->handle = last_handle;
				hndl->next = head;
				hndl->type = HANDLE_FILE;
				head = hndl;
				mutex_unlock(&q_locked);
				msg.arg[0] = last_handle;
				msg.arg[1] = TMPFS_BUF_SIZE;
				msg.arg[2] = NO_ERR;
				msg.arg[3] = hndl->target->size;
				msg.send_size = 0;
				break;
			}
			break;
		}
		case FS_CMD_STAT: {
			tmpfs_file_t *f;
			buffer[msg.recv_size] = 0;
			if(tfs->locate_file(buffer, &f) < 0) {
				msg.send_size = 0;
				msg.arg[2] =ERR_NO_SUCH_FILE;
				break;
			}
			tfs->stat(f, statbuf);
			msg.arg[1] = TMPFS_BUF_SIZE;
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
			tfs->stat(h->target, statbuf);
			msg.arg[1] = TMPFS_BUF_SIZE;
			msg.arg[2] = NO_ERR;
			msg.send_size = sizeof(struct stat);
			msg.send_buf = statbuf;
			break;
		}
		case FS_CMD_READ: {
			size_t size = msg.send_size;
			if(size > TMPFS_BUF_SIZE)
				size = TMPFS_BUF_SIZE;
			handle_t *h = resolve_handle(msg.arg[1], HANDLE_FILE);
			if(!h) {
				msg.arg[2] = ERR_NO_SUCH_FILE;
				msg.arg[1] = 0;
				msg.send_size = 0;
				break;
			}
			h->touched = uptime();
			unsigned int readed = tfs->read(h->target, buffer,  size, msg.arg[2]);
			if(readed < size) 
				msg.arg[2] = ERR_EOF;
			else
				msg.arg[2] = NO_ERR;
			msg.arg[0] = readed;
			msg.arg[1] = h->target->size;
			msg.send_size = msg.arg[0];
			msg.send_buf = buffer;
			break;
		}
		case FS_CMD_WRITE: {
			size_t size = msg.recv_size;
			if(size > TMPFS_BUF_SIZE)
				size = TMPFS_BUF_SIZE;
			handle_t *h = resolve_handle(msg.arg[1], HANDLE_FILE);
			if(!h) {
				msg.arg[2] = ERR_NO_SUCH_FILE;
				msg.arg[1] = 0;
				msg.send_size = 0;
				break;
			}
			h->touched = uptime();
			msg.arg[0] =  tfs->write(h->target, buffer,  size, msg.arg[2]);
			msg.arg[1] = h->target->size;
			msg.arg[2] = NO_ERR;

			msg.send_size = 0;
			break;
		}
		case FS_CMD_DIRCLOSE:
		case FS_CMD_CLOSE:
			while(!mutex_try_lock(&q_locked))
				sched_yield();
			for(handle_t *p = head, *prev = NULL; p; prev = p, p = p->next) {
				if(p->handle == msg.arg[1] && p->type == HANDLE_FILE) {
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
			if(tfs->open_dir(buffer, &hndl->target) < 0) {
				msg.arg[2] = ERR_NO_SUCH_FILE;
				msg.arg[1] = 0;
				msg.send_size = 0;
				break;
			}
			while(!mutex_try_lock(&q_locked))
 				 sched_yield();

			last_handle ++;
			hndl->touched = uptime();
			hndl->handle = last_handle;
			hndl->next = head;
			hndl->type = HANDLE_DIR;
			head = hndl;
			mutex_unlock(&q_locked);
			msg.arg[0] = last_handle;
			msg.arg[1] = hndl->target->size;
			msg.arg[2] = NO_ERR;
			msg.send_size = 0;
			break;
		}
		case FS_CMD_DIRREAD: {
			handle_t *h = resolve_handle(msg.arg[1], HANDLE_DIR);
			if(!h) {
				msg.arg[2] = ERR_NO_SUCH_FILE;
				msg.send_size = 0;
				break;
			}
			tmpfs_file_t *file;
			if(tfs->get_file_by_offset(h->target, &file, msg.arg[2]) < 0) {
				msg.arg[2] = ERR_EOF;
				msg.send_size = 0;
				break;
			}
			dent.d_ino = (u32_t)file;
			dent.d_reclen = strlen(file->name);
			strcpy(dent.d_name, file->name);
			msg.send_size = sizeof(struct dirent);
			msg.send_buf = &dent;
			msg.arg[2] = NO_ERR;
			break;			
		}
		case FS_CMD_UNLINK: {
			buffer[msg.recv_size] = 0;
			tfs->unlink(buffer);
			msg.arg[2] = NO_ERR;
			msg.send_size = 0;
			break;
		}
		case FS_CMD_POSIX_ACCESS:
			buffer[msg.recv_size] = 0;
			if(tfs->posix_access(buffer) < 0) {
				msg.arg[2] = ERR_NO_SUCH_FILE;
				msg.send_size = 0;
				break;
			}
			msg.arg[2] = NO_ERR;
			msg.send_size = 0;
			break;
		case FS_CMD_MKDIR:
			buffer[msg.recv_size] = 0;
			printf("buffer %s size %d\n", buffer, msg.recv_size);
			if(tfs->mkdir(buffer) < 0) {
				msg.arg[2] = ERR_NO_SUCH_FILE;
				msg.send_size = 0;
				break;
			}
			msg.arg[2] = NO_ERR;
			msg.send_size = 0;
			break;				
		default:
			printf("tmpfs: unknown command %u %u %u %u\n", msg.arg[0], msg.arg[1], msg.arg[2], msg.arg[3]);
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
		msg.flags = 0;
		alarm(60000);
		receive(&msg);
		reply(&msg);
		while(!mutex_try_lock(&q_locked))
	 		sched_yield();
		for(handle_t *ptr = head, *prev = NULL; ptr; prev = ptr, ptr = ptr->next) {
			if(ptr->touched < uptime() - 60000) {
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


