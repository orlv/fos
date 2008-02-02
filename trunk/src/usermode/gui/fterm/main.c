/*
 * Copyright (c) 2008 Sergey Gridassov.
 */

#include <fos/fos.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <fgs/fgs.h>
#include <sched.h>
#include <string.h>
#include <mutex.h>
#include <fos/message.h>
#define RECV_BUF_SIZE 2048

volatile char *tty = NULL;
volatile int gui_ready = 0;
int pos = 0;
volatile int havechars = 0;
volatile int winhandle = 0;
volatile u8_t keyptr = 0;
volatile char *keychars;
volatile int need_redraw = 0;
int mainthread;


typedef struct waiting_list {
	struct waiting_list *next;
	int tid;
	int size;
} waiting_list;

mutex_t key_locked = 0;
waiting_list *keyboard = NULL;

THREAD(redraw) {
	while(1) {
		if(need_redraw) {
			RefreshWindow(winhandle);
			need_redraw--;
		} else
			sched_yield();
	}
}


int tty_read(char *buf, int max) {
	int readed = 0;
	for(;readed < max && havechars; readed++) {
		buf[readed] = keychars[--keyptr];
		havechars--;
	}
	return readed;
}

void check_old() {
	char replybuf[128];
	while(!mutex_try_lock(key_locked)) sched_yield();
	struct message mreply;
	for(waiting_list *ptr = keyboard, *prev = NULL, *next = NULL; ptr; prev = ptr, ptr = next) {
		mreply.arg[0] = tty_read(replybuf, ptr->size < 128 ? ptr->size : 128);
		mreply.arg[2] = ERR_EOF;
		mreply.flags = 0;
		mreply.recv_size = 0;
		mreply.send_size = mreply.arg[0];
		mreply.send_buf = replybuf;
		mreply.tid = ptr->tid;
		reply(&mreply);
		if(prev)
			prev->next = ptr->next;
		else
			keyboard = ptr->next;
		next = ptr->next;
		free(ptr);
	}
	mutex_unlock(key_locked);
}


void scroll() {
	off_t i;

	for (i = 0; i < 80 * (25 - 1); i++) {
		tty[i] = tty[i + 80];
	}
	for (; i < 80 * 25; i++) {
		tty[i] = 0;
	}
	ShiftWindowUp(winhandle, 16);

	pos -= 80;
}

void tty_putc(unsigned char ch) {
	line(winhandle, (pos % 80) * 8, (pos / 80) * 16, (pos % 80) * 8, (pos / 80) * 16 + 15, 0x000000);
	char buf[2];
	buf[1] = 0;
	switch(ch) {
	case '\n':
		pos += 80;
		pos -= pos % 80;
		break;
	case 0x08:
		buf[0] = ' ';
		tty[pos] = ' ';
		pos--;
		rect(winhandle, (pos % 80) * 8, (pos / 80) * 16, 8 * 2, 16, 0);
		break;
	default:
		tty[pos] = ch;
		buf[0] = ch;
		rect(winhandle, (pos % 80) * 8, (pos / 80) * 16, 8, 16, 0);
		pstring(winhandle, (pos % 80) * 8, (pos / 80) * 16, 0xbfbfbf, buf);
		pos++;
		break;
	}
	if(pos >= 80 * 25) scroll();
	line(winhandle, (pos % 80) * 8, (pos / 80) * 16, (pos % 80) * 8, (pos / 80) * 16 + 15, 0xFFFFFF);
}

int tty_write(char *buf, int count) {
	for(int i = 0; i < count; i++, buf++)
		tty_putc(*buf);
	need_redraw++;
	return count;
}

void gui_thread() {
	GUIInit();
	int tmp;
	winhandle = CreateWindow(0, 0, 80 * 8, 25 * 16, "FTerm", WC_WINDOW, &tmp);
	SetVisible(winhandle, 1);
	rect(winhandle, 0, 0, 80 * 8, 25 * 16, 0);
	RefreshWindow(winhandle);
	tty = malloc(80 * 25 + 1);
	keychars = malloc(256);
	memset((char *)tty, ' ', 80 * 25);
	gui_ready = 1;
	int class, handle, a0, a1, a2, a3;
	struct message notify;
	notify.tid = mainthread;
	notify.send_size = 0;
	notify.recv_size = 0;
	notify.flags = MSG_ASYNC;
	notify.arg[0] = 0x55AA55AA;
	while (1) {
		WaitEvent(&class, &handle, &a0, &a1, &a2, &a3);
		switch (class) {
		case EV_WINCLOSE:
			DestroyWindow(winhandle);
			GuiEnd();
			exit(1);
		case EV_KEY:
			if(a1) break;
			if(keyptr == 255) keyptr = 0;
			keychars[keyptr++] = a0;
			havechars++;
			send(&notify);
			break;
		}
	}
}

void StartChild(char *point, char *child) {
	setenv("STDOUT", point, 1);
	setenv("STDIN", point, 1);
	setenv("STDERR", point, 1);
	exec(child, NULL);
}

int main(int argc, char *argv[]) {
	thread_create((off_t) gui_thread);
	thread_create((off_t) redraw);
	srandom(uptime());
	char *name = malloc(32);
	mainthread = my_tid();
	int handle;
	do {
		snprintf(name, 32, "/dev/fterm%x", random());
		handle = open(name, 0);
		if(handle > 0)
			close(handle);
	} while(handle > 0);
	do {} while(!gui_ready);
	resmgr_attach(name);
	struct message msg;
	char *buffer = malloc(RECV_BUF_SIZE);
	StartChild(name, "/bin/shell");
	while (1) {
		msg.tid = 0;
		msg.recv_size = RECV_BUF_SIZE;
		msg.recv_buf = buffer;
		receive(&msg);
		switch (msg.arg[0]) {
		case FS_CMD_ACCESS:
			msg.arg[0] = 1;
			msg.arg[1] = RECV_BUF_SIZE;
			msg.arg[2] = NO_ERR;
			msg.send_size = 0;
			reply(&msg);
			break;

		case FS_CMD_WRITE:
			msg.arg[0] = tty_write(buffer, msg.recv_size);
			msg.arg[2] = NO_ERR;
			msg.send_size = 0;
			reply(&msg);
			break;

		case FS_CMD_READ:
			while(!mutex_try_lock(key_locked)) sched_yield();
			if(!havechars) {
				waiting_list *new = malloc(sizeof(waiting_list));
				new->next = keyboard;
				new->tid = msg.tid;
				new->size = msg.send_size;
				keyboard = new;
				mutex_unlock(key_locked);
				break;
			}
	
			msg.arg[0] = tty_read(buffer, msg.send_size);
			msg.arg[2] = NO_ERR;
			msg.send_size = msg.arg[0];
			msg.send_buf = buffer;
			mutex_unlock(key_locked);
			reply(&msg);
			break;
		case  0x55AA55AA:
			check_old();
			reply(&msg);
			break;			
		default:
			msg.arg[0] = 0;
			msg.arg[2] = ERR_UNKNOWN_CMD;
			reply(&msg);
		}
	}
	return 0;
}

