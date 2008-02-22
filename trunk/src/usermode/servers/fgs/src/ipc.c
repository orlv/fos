#include <fos/message.h>
#include <fos/fos.h>
#include <stdio.h>
#include <string.h>
#include <list.h>
#include <stdlib.h>
#include <private/windowing.h>

extern context_t screen;
extern volatile int ready_counter;

typedef struct event_q {
  struct list_head list;
  int class;
  int handle;
  int a0;
  int a1;
  int a2;
  int a3;
} event_q_t;

typedef struct proc_q {
  struct list_head list;
  int waiting;
  int tid;
  event_q_t events;
  int timer_interval;
  int timer_counter;
} proc_t;

static mutex_t q_locked = 0;

proc_t proc_head = {
  .list = LIST_HEAD_INIT(proc_head.list),
  .waiting = 0,
  .tid = 0,
  .events.list = LIST_HEAD_INIT(proc_head.events.list),
  .events.class = 0,
  .events.handle = 0,
  .events.a0 = 0,
  .events.a1 = 0,
  .events.a2 = 0,
  .events.a3 = 0
};

static tid_t ipc_thread;

void PostEvent(int tid, int handle, int class, int a0, int a1, int a2, int a3)
{
  while (!mutex_try_lock(q_locked))
    sched_yield();
  struct list_head *entry;
  proc_t *p = &proc_head;

  event_q_t *ev = malloc(sizeof(event_q_t));

  ev->class = class;
  ev->handle = handle;
  ev->a0 = a0;
  ev->a1 = a1;
  ev->a2 = a2;
  ev->a3 = a3;

  list_for_each(entry, &proc_head.list) {
    p = list_entry(entry, proc_t, list);
    if (p->tid == tid)
      break;
  }
  if (p->tid != tid) {		/* запись процесса не найдена, создаем */
    p = malloc(sizeof(proc_t));
    p->waiting = 0;
    p->timer_interval = 0;
    INIT_LIST_HEAD(&p->events.list);
    p->tid = tid;
    list_add_tail(&p->list, &proc_head.list);
  }

  list_add_tail(&ev->list, &p->events.list);
  mutex_unlock(q_locked);
  if(ipc_thread) {
    struct message msg; 
    msg.tid = ipc_thread;
    msg.arg[0] = FS_CMD_ACCESS;
    msg.flags = MSG_ASYNC;
    msg.send_size = 0;
    msg.recv_size = 0;
    send(&msg);
  } else
    printf("Warning: IPC thread not started while sending events\n"
           " BUG!\n");
}

void EventsThread()
{
  struct message msg;
  char *buffer = malloc(sizeof(create_win_t) + MAX_TITLE_LEN);
  int buf_size = sizeof(create_win_t) + MAX_TITLE_LEN;

  resmgr_attach("/dev/pgs/main");
  ready_counter--;
  while (1) {
    msg.tid = 0;
    msg.recv_buf = buffer;
    msg.flags = 0;		//MSG_MEM_SHARE;
    msg.recv_size = buf_size;
    receive(&msg);

    if (msg.tid != 0)
      switch (msg.arg[0]) {

      case FS_CMD_ACCESS:
	msg.arg[0] = 1;
	msg.arg[1] = sizeof(create_win_t) + MAX_TITLE_LEN;
	msg.arg[2] = NO_ERR;
	msg.send_size = 0;
	reply(&msg);
	break;
      case WIN_CMD_CREATEWINDOW:{
	  create_win_t *win = (create_win_t *) buffer;
	  struct win_info wi;
	  char *caption = buffer + sizeof(create_win_t);

	  wi.handle = CreateWindow(win->x, win->y, msg.tid, win->w, win->h, caption, win->class);
	  window_t *w = GetWindowInfo(wi.handle);

	  wi.bpp = w->context->bpp;
	  msg.arg[2] = NO_ERR;
	  if (win->class & WC_NODECORATIONS) {
	    wi.margin_up = 0;
	    wi.margin_left = 0;
	    wi.margin_right = 0;
	    wi.margin_down = 0;
	  } else {
	    wi.margin_up = 21;
	    wi.margin_left = 3;
	    wi.margin_right = 3;
	    wi.margin_down = 3;
	  }
	  msg.send_size = sizeof(wi);
	  msg.send_buf = &wi;
	  msg.flags = 0;
	  reply(&msg);
	  break;
	}
      case WIN_CMD_DESTROYWINDOW:
	DestroyWindow(msg.arg[1]);
	msg.arg[2] = NO_ERR;
	msg.send_size = 0;
	msg.flags = 0;
	reply(&msg);
	break;
      case WIN_CMD_SETVISIBLE:
	SetVisible(msg.arg[1], msg.arg[2]);
	msg.arg[2] = NO_ERR;
	msg.send_size = 0;
	msg.flags = 0;
	reply(&msg);
	break;
      case WIN_CMD_REFRESHWINDOW:
	RefreshWindow(msg.arg[1]);
	msg.arg[2] = NO_ERR;
	msg.send_size = 0;
	msg.flags = 0;
	reply(&msg);
	break;
      case WIN_CMD_WAIT_EVENT:{
	  while (!mutex_try_lock(q_locked))
	    sched_yield();

	  struct list_head *entry;
	  proc_t *p = &proc_head;

	  list_for_each(entry, &proc_head.list) {
	    p = list_entry(entry, proc_t, list);
	    if (p->tid == msg.tid)
	      break;
	  }

	  if (p->tid != msg.tid) {	/* запись процесса не найдена, создаем */
	    p = malloc(sizeof(proc_t));
	    p->waiting = 1;
	    INIT_LIST_HEAD(&p->events.list);
	    p->tid = msg.tid;
	    list_add_tail(&p->list, &proc_head.list);
	  } else {
	    if (!list_empty(&p->events.list)) {
	      event_q_t *ev = list_entry(p->events.list.next, event_q_t, list);

	      list_del(&ev->list);
	      msg.send_size = sizeof(event_q_t);
	      msg.send_buf = ev;
	      msg.flags = 0;
	      reply(&msg);
	      free(ev);
	      p->waiting = 0;
	    } else
	      p->waiting = 1;
	  }
	  mutex_unlock(q_locked);
	  break;
	}

      case WIN_CMD_CLEANUP:{
	  while (!mutex_try_lock(q_locked))
	    sched_yield();

	  struct list_head *entry;
	  proc_t *p;

	  list_for_each(entry, &proc_head.list) {
	    p = list_entry(entry, proc_t, list);
	    if (p->tid == msg.tid) {
	      list_del(entry);
	      list_for_each(entry, &p->events.list) {
		event_q_t *ev = list_entry(entry, event_q_t, list);

		free(ev);
	      }
	      free(p);
	      break;
	    }
	  }
	  msg.send_size = 0;
	  msg.flags = 0;
	  reply(&msg);
	  mutex_unlock(q_locked);
	  break;
	}
      case WIN_CMD_SCREEN_INFO:
	msg.arg[0] = screen.w;
	msg.arg[1] = screen.h;
	msg.arg[2] = screen.bpp;
	msg.send_size = 0;
	msg.flags = 0;
	reply(&msg);
	break;
      case WIN_CMD_SETFOCUS:
        SetFocusTo(msg.arg[1]);
	msg.send_size = 0;
	msg.flags = 0;
	reply(&msg);
	break;    
      case WIN_CMD_GETTITLE:
        msg.arg[0] = GetWindowTitle(msg.arg[1], buffer);
        msg.send_size = strlen(buffer);
        msg.send_buf = buffer;
	msg.flags = 0;
	reply(&msg);
	break; 
      case FS_CMD_CLOSE:
	msg.arg[0] = 0;
	msg.arg[2] = NO_ERR;
	msg.send_size = 0;
	reply(&msg);
	break;
      default:
	printf("main events thread received message: %u %u %u %u\n", msg.arg[0], msg.arg[1], msg.arg[2], msg.arg[3]);
	msg.arg[0] = 0;
	msg.arg[2] = ERR_UNKNOWN_CMD;
	msg.send_size = 0;
	reply(&msg);
      }

    struct list_head *entry;
    proc_t *p;

    while (!mutex_try_lock(q_locked))
      sched_yield();

    list_for_each(entry, &proc_head.list) {
      p = list_entry(entry, proc_t, list);
      if (p->waiting && !list_empty(&p->events.list)) {
	event_q_t *ev = list_entry(p->events.list.next, event_q_t, list);

	list_del(&ev->list);
	msg.send_size = sizeof(event_q_t);
	msg.send_buf = ev;
	msg.flags = 0;
	msg.tid = p->tid;
	reply(&msg);
	free(ev);
	p->waiting = 0;
      }
    }
    mutex_unlock(q_locked);
  }
}


void MappingThread()
{
  resmgr_attach("/dev/pgs/mapping");
  struct message msg;
  ready_counter--;
  while (1) {
    msg.tid = 0;
    msg.recv_buf = NULL;
    msg.flags = MSG_MEM_SHARE;
    msg.recv_size = screen.w * screen.h * screen.bpp;
    receive(&msg);
    switch (msg.arg[0]) {
    case FS_CMD_ACCESS:
      msg.arg[0] = 1;
      msg.arg[1] = screen.w * screen.h * screen.bpp;
      msg.arg[2] = NO_ERR;
      msg.send_size = 0;
      reply(&msg);
      break;
    case WIN_CMD_MAPBUF:{
	window_t *w = GetWindowInfo(msg.arg[1]);

	if (msg.recv_size < w->context->w * w->context->h * w->context->bpp) {
	  printf("invalid size: %d, must be %d\n", msg.recv_size, w->context->w * w->context->h * w->context->bpp);
	  break;
	}
	msg.send_size = 0;
	msg.flags = 0;
	reply(&msg);
	w->context->data = msg.recv_buf;
	WindowMapped(w);

	break;
      }
    case FS_CMD_CLOSE:
      msg.arg[0] = 0;
      msg.arg[2] = NO_ERR;
      msg.send_size = 0;
      reply(&msg);
      break;
    default:
      printf("message: %u %u %u %u\n", msg.arg[0], msg.arg[1], msg.arg[2], msg.arg[3]);
      msg.arg[0] = 0;
      msg.arg[2] = ERR_UNKNOWN_CMD;
      msg.send_size = 0;
      reply(&msg);
    }
  }
}

void StartIPC() {
  ipc_thread = thread_create((off_t) & EventsThread);
  thread_create((off_t) & MappingThread);
}
