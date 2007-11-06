/*
  (C) 2007 Serge Gridasov
  (Mon Nov  5 23:04:21 2007) Oleg Fedorov: переделал по-своему
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <fos/fos.h>
#include <fos/message.h>
#include <sched.h>
#include <list.h>
#include <gui/types.h>
#include <mutex.h>

void event_handler(event_t *event);

struct mouse_pos {
  int dx;
  int dy;
  int dz;
  int b;
};

extern mode_definition_t __current_mode;

typedef struct event_q{
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
} proc_t;

mutex_t q_locked = 0;

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

void DestroyWindow(int handle);

int CreateWindow(int tid, int x, int y, int w, int h, char *caption, int class);

void PostEvent(int tid, int handle, int class, int a0, int a1, int a2, int a3)
{
  while(!mutex_try_lock(q_locked))
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
  
  list_for_each(entry, &proc_head.list){
    p = list_entry(entry, proc_t, list);
    if(p->tid == tid)
      break;
  }
  
  if(p->tid != tid) { /* запись процесса не найдена, создаем */
    p = malloc(sizeof(proc_t));
    p->waiting = 0;
    INIT_LIST_HEAD(&p->events.list);
    p->tid = tid;
    list_add_tail(&p->list, &proc_head.list);
  }

  list_add_tail(&ev->list, &p->events.list);
  mutex_unlock(q_locked);
}

void EventsThread()
{
  struct message msg;
  char *buffer = malloc( sizeof(create_win_t) + MAX_TITLE_LEN);
  resmgr_attach("/dev/pgs");
  while(1) {
    msg.tid = _MSG_SENDER_ANY;
    msg.recv_buf  = buffer;
    msg.flags = 0;
    msg.recv_size = sizeof(create_win_t) + MAX_TITLE_LEN;
    alarm(50);
    receive(&msg);
    alarm(0);

    if(msg.tid != _MSG_SENDER_SIGNAL)
      switch(msg.a0) {

      case FS_CMD_ACCESS:
	msg.a0 = 1;
	msg.a1 = sizeof(create_win_t) + MAX_TITLE_LEN;
	msg.a2 = NO_ERR;
	msg.send_size = 0;
	reply(&msg);
	break;

      case WIN_CMD_CREATEWINDOW: {
	create_win_t *win = (create_win_t *) buffer;
	char *caption = buffer + sizeof(create_win_t);
	msg.a0 = CreateWindow(msg.tid, win->x, win->y, win->w, win->h, caption, win->class);
	msg.a2 = NO_ERR;
	msg.send_size = 0;
	msg.flags = 0;
	reply(&msg);
	break;
      }

      case WIN_CMD_DESTROYWINDOW:
	DestroyWindow(msg.a1);
	msg.a2 = NO_ERR;
	msg.send_size = 0;
	msg.flags = 0;
	reply(&msg);
	break;

      case WIN_CMD_WAIT_EVENT: {
	while(!mutex_try_lock(q_locked))
	  sched_yield();

	struct list_head *entry;
	proc_t *p = &proc_head;

	list_for_each(entry, &proc_head.list){
	  p = list_entry(entry, proc_t, list);
	  if(p->tid == msg.tid)
	    break;
	}

	if(p->tid != msg.tid) { /* запись процесса не найдена, создаем */
	  p = malloc(sizeof(proc_t));
	  p->waiting = 1;
	  INIT_LIST_HEAD(&p->events.list);
	  p->tid = msg.tid;
	  list_add_tail(&p->list, &proc_head.list);
	} else {
	  if(!list_empty(&p->events.list)) {
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

      case WIN_CMD_CLEANUP: {
	while(!mutex_try_lock(q_locked))
	  sched_yield();

	struct list_head *entry;
	proc_t *p;

	list_for_each(entry, &proc_head.list){
	  p = list_entry(entry, proc_t, list);
	  if(p->tid == msg.tid){
	    list_del(entry);
	    list_for_each(entry, &p->events.list){
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
	
      default:
	printf("message: %u %u %u %u\n", msg.a0, msg.a1, msg.a2, msg.a3);
	msg.a0 = 0;
	msg.a2 = ERR_UNKNOWN_CMD;
	msg.send_size = 0;
	reply(&msg);
      }

    struct list_head *entry;
    proc_t *p;

    while(!mutex_try_lock(q_locked))
      sched_yield();

    list_for_each(entry, &proc_head.list){
      p = list_entry(entry, proc_t, list);
      if(p->waiting && !list_empty(&p->events.list)) {
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
  printf("WARNING: EXIT FROM INFINITE LOOP!\n");
}

void mouse_thread()
{
  int x = 0;
  int y = 0;
  int oldx = 0;
  int oldy = 0;
  int oldb = 0;
  int psaux;
  do {
    psaux = open("/dev/psaux", 0);
    sched_yield();
  } while(!psaux);
  struct mouse_pos move;
  while(1) {
    read(psaux, &move, sizeof(struct mouse_pos));
    x += move.dx;
    y += move.dy;
    if(x < 0) x = 0;
    if(y < 0) y = 0;
    if(x > __current_mode.width) x = __current_mode.width;
    if(y > __current_mode.height) y = __current_mode.height;
    if(x != oldx || y != oldy) { // мышь сдвинули
      oldx = x;
      oldy = y;
      mousemove_event_t mouse = { x, y};
      event_t event = { EVENT_TYPE_MOUSEMOVE, &mouse };
      event_handler(&event);
    }
    if(move.b && !oldb) { // кнопку нажали
      event_t event = { EVENT_TYPE_MOUSEDOWN, NULL };
      event_handler(&event);
      oldb = move.b;
    }
    if(!move.b && oldb) { // отпустили
      event_t event = { EVENT_TYPE_MOUSEUP, NULL };
      event_handler(&event);
      oldb = move.b;	
    }
    
  }
} 

void StartEventHandling()
{
  thread_create((off_t)&mouse_thread);
  thread_create((off_t)&EventsThread);
}