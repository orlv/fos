#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <fos/fos.h>
#include <fos/message.h>
#include <sched.h>

#include <gui/types.h>
void event_handler(event_t *event);
struct mouse_pos {
	int dx;
	int dy;
	int dz;
	int b;
};
extern mode_definition_t __current_mode;
typedef struct event_q{
	int class;
	int handle;
	int a0;
	int a1;
	int a2;
	int a3;
	struct event_q * next;
} event_q_t;
typedef struct proc_q {
	int waiting;
	int tid;
	event_q_t *events;
	struct proc_q *next;
} proc_t;
volatile int q_locked = 0;
proc_t *proc_head;
void DestroyWindow(int handle);
int CreateWindow(int tid, int x, int y, int w, int h, char *caption, int class);
void PostEvent(int tid, int handle, int class, int a0, int a1, int a2, int a3) {
	q_locked = 1;
	for(proc_t *p = proc_head; p; p = p->next) {
		if(p->tid == tid) {
			event_q_t *ev = malloc(sizeof(event_q_t));
			ev->class = class;
			ev->handle = handle;
			ev->a0 = a0;
			ev->a1 = a1;
			ev->a2 = a2;
			ev->a3 = a3;
			ev->next = p->events;
			p->events = ev;
			q_locked = 0;
			return;
		}
	}
	// процесса нет, ааа
	proc_t *proc = malloc(sizeof(proc_t));
	proc->waiting = 0;
	proc->tid = tid;
	proc->next = proc_head;
	proc_head = proc;
	event_q_t *ev = malloc(sizeof(event_q_t));
	ev->class = class;
	ev->handle = handle;
	ev->a0 = a0;
	ev->a1 = a1;
	ev->a2 = a2;
	ev->a3 = a3;
	ev->next = NULL;
	proc->events = ev;
	q_locked = 0;
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
    alarm(100);
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
	reply(&msg);
	break;
      }
      case WIN_CMD_DESTROYWINDOW:
	DestroyWindow(msg.a1);
	msg.a2 = NO_ERR;
	msg.send_size = 0;
	reply(&msg);
	break;
      case WIN_CMD_WAIT_EVENT: {
	proc_t *p;
	while(q_locked) sched_yield();
	for(p = proc_head; p; p = p->next) {
	  if(p->tid == msg.tid) {
	    if(p->events != NULL) {
	      event_q_t *ev = p->events;
	      p->events = ev->next;
	      msg.send_size = sizeof(event_q_t);
	      msg.send_buf = ev;
	      reply(&msg);
	      free(ev);
	      p->waiting = 0;
	      
	    } else 
	      p->waiting = 1;
	    break;
	  }
	}
	if(!p) { // процесс не найден.
	  proc_t *ev = malloc(sizeof(proc_t));
	  ev->waiting = 1;
	  ev->events = NULL;
	  ev->next = proc_head;
	  ev->tid = msg.tid;
	  proc_head = ev;
	}
	break;
      }
      case WIN_CMD_CLEANUP: {
	// FIXME: тут виснет.
	break;
	proc_t *n = NULL;
	for(proc_t *p = proc_head; p;) {
	  n = p->next;
	  if(p->tid == msg.tid) {
	    if(p->events != NULL) {
	      event_q_t * n;
	      for(event_q_t *ev = p->events; ev; n = ev->next) {
		free(ev);
		ev = n;
	      }
	    }
	    if(n)
	      n->next = p->next;
	    free(p);
	  }
	  p = n;
	}
	break;
      }
      default:
	printf("message: %u %u %u %u\n", msg.a0, msg.a1, msg.a2, msg.a3);
	msg.a0 = 0;
	msg.a2 = ERR_UNKNOWN_CMD;
	msg.send_size = 0;
	reply(&msg);
      }
    while(q_locked) sched_yield();
    for(proc_t *p = proc_head; p; p = p->next) {
      if(p->waiting && p->events) {
	event_q_t *ev = p->events;
	p->events = ev->next;
	msg.recv_size = 0;
	msg.flags = 0;
	msg.send_size = sizeof(event_q_t);
			msg.send_buf = ev;
			msg.tid = p->tid;
			reply(&msg);
			free(ev);
			p->waiting = 0;
      }
    }
  }
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
  thread_create(&mouse_thread);
  thread_create(&EventsThread);
}
