/*
 * Copyright (C) 2008 Sergey Gridassov
 */

#include <fos/fos.h>
#include <fos/message.h>
#include <fos/fs.h>
#include <c++/list.h>
#include <mutex.h>
#include <stdlib.h>
#include <sched.h>
#include <stdio.h>

#include "assert.h"
#include "ipc.h"

typedef struct {
	unsigned int	ev_class;
	unsigned int	global_code;
	unsigned int	handle;
	unsigned int	a0;
	unsigned int	a1;
	unsigned int	a2;
	unsigned int	a3;
} event_q_t;

typedef struct {
	bool	waiting;
	tid_t	tid;
	u32_t	global_mask;
	List	<event_q_t *> *events;
} proc_t;

static mutex_t q_locked = 0;

static List <proc_t *> *proc_head;

static volatile tid_t ipc_thread = 0;

static inline List <proc_t *> *AllocateProcess(tid_t tid, bool waiting) {
	proc_t *item = new proc_t;
	assert(item != NULL);
	item->waiting = waiting;
	item->tid = tid;
	item->global_mask = 0;
	item->events = new List<event_q_t*>();
	assert(item->events != NULL);
	List <proc_t *> *ptr = proc_head->add(item);
	assert(ptr != NULL);
	return ptr;
}


void PostEvent(	tid_t tid, unsigned int handle,	unsigned int ev_class,	unsigned int global,
		unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3) {
	while (!mutex_try_lock(&q_locked))
		sched_yield();
	if(!global) {
		event_q_t *ev = new event_q_t;

		assert(ev != NULL);

		ev->ev_class = ev_class;
		ev->handle = handle;
		ev->a0 = a0;
		ev->a1 = a1;
		ev->a2 = a2;
		ev->a3 = a3;
		ev->global_code = global;
	

		List <proc_t *> *ptr = proc_head;
		list_for_each(ptr, proc_head)
			if(ptr->item != NULL && ptr->item->tid == tid)
				break;

		if(ptr->item->tid != tid) {
			ptr = AllocateProcess(tid, false);
		}
		ptr->item->events->add(ev);
	} else {
		List <proc_t *> *ptr = proc_head;
		list_for_each(ptr, proc_head) {
			if(ptr->item && ptr->item->global_mask & global) {
				event_q_t *ev = new event_q_t;

				assert(ev != NULL);

				ev->ev_class = ev_class;
				ev->handle = handle;
				ev->a0 = a0;
				ev->a1 = a1;
				ev->a2 = a2;
				ev->a3 = a3;
				ev->global_code = global;
				ptr->item->events->add(ev);
			}
		}
	}
	mutex_unlock(&q_locked);

	assert(ipc_thread != 0);

	struct message msg;
	msg.tid = ipc_thread;
	msg.arg[0] = WIN_CMD_INTERNAL_NOTIFY;
	msg.flags = 0;
	msg.send_size = 0;
	msg.recv_size = 0;
	send(&msg);
}

static void IPCThread() {
	struct message msg;
	int buf_sz = 256;		// FIXME: заменить на реальные
	char *buf = new char[256];	// значения
	resmgr_attach("/dev/gwinsy/ipc");
	ipc_thread = my_tid();
	while(1) {
		msg.tid = 0;
		msg.recv_size = buf_sz;
		msg.recv_buf = buf;
		receive(&msg);
		switch(msg.arg[0]) {
		case FS_CMD_ACCESS:
			msg.arg[0] = 1;
			msg.arg[1] = buf_sz;
			msg.arg[2] = NO_ERR;
			msg.send_size = 0;
			reply(&msg);
			break;
		case WIN_CMD_GET_EVMASK: {
			while(!mutex_try_lock(&q_locked))
				sched_yield();

			List <proc_t *> *ptr = NULL;
			list_for_each(ptr, proc_head) {
				if(ptr->item != NULL && ptr->item->tid == msg.tid)
					break;
			}
			
			if(ptr->item == NULL || ptr->item->tid != msg.tid) {
				ptr = AllocateProcess(msg.tid, true);
			} 
			msg.arg[0] = ptr->item->global_mask;
			mutex_unlock(&q_locked);
			msg.send_size = 0;
			reply(&msg);
			break;
		}

		case WIN_CMD_SET_EVMASK: {
			while(!mutex_try_lock(&q_locked))
				sched_yield();

			List <proc_t *> *ptr = proc_head;
			list_for_each(ptr, proc_head) {
				if(ptr->item != NULL && ptr->item->tid == msg.tid)
					break;
			}
			
			if(ptr->item == NULL || ptr->item->tid != msg.tid) {
				ptr = AllocateProcess(msg.tid, true);
			} 
			ptr->item->global_mask = msg.arg[1];
			mutex_unlock(&q_locked);
			msg.send_size = 0;
			reply(&msg);
			break;
		}

		case WIN_CMD_WAIT_EVENT: {
			while(!mutex_try_lock(&q_locked))
				sched_yield();
			List <proc_t *> *ptr = proc_head;
			list_for_each(ptr, proc_head) {
				if(ptr->item != NULL && ptr->item->tid == msg.tid)
					break;
			}
			if(ptr->item == NULL || ptr->item->tid != msg.tid) {
				ptr = AllocateProcess(msg.tid, true);
			} else {
				if(!ptr->item->events->empty()) {
					event_q_t *ev = ptr->item->events->next->item;
					delete ptr->item->events->next;
					msg.send_size = sizeof(event_q_t);
					msg.send_buf = ev;
					msg.flags = 0;
					reply(&msg);
					delete ev;
					ptr->item->waiting = false;
				} else
					ptr->item->waiting = true;
			}
			mutex_unlock(&q_locked);
			break;
		}
		case WIN_CMD_INTERNAL_NOTIFY: {
			reply(&msg);
			while(!mutex_try_lock(&q_locked))
				sched_yield();

			List <proc_t *> *ptr;

			list_for_each(ptr, proc_head) {
				if(ptr->item && ptr->item->waiting && !ptr->item->events->empty()) {
					event_q_t *ev = ptr->item->events->next->item;
					delete ptr->item->events->next;
					msg.send_size = sizeof(event_q_t);
					msg.send_buf = ev;
					msg.flags = 0;
					msg.tid = ptr->item->tid;
					reply(&msg);
					delete ev;
					ptr->item->waiting = false;
				}
			}

			mutex_unlock(&q_locked);
			break;
		}
		default:
			printf("main events thread received message: %u %u %u %u\n", msg.arg[0], msg.arg[1], msg.arg[2], msg.arg[3]);
			msg.arg[0] = 0;
			msg.arg[2] = ERR_UNKNOWN_METHOD;
			msg.send_size = 0;
			reply(&msg);
		}
	}	
}

void ipc_init() {
	proc_head = new List<proc_t *>();
	thread_create((off_t) & IPCThread, 0);
	while(ipc_thread == 0)
		sched_yield();
}
