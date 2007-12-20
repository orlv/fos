/*
 *         FOS Graphics System
 * Copyright (c) 2007 Sergey Gridassov
 * Modifications by   Oleg Fedorov
 * Original cursor handling algorithm 
 *  by Sadovnikov Vladimir
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
#include <mutex.h>

#include <private/types.h>
#include <private/cursor.h>
#include <private/windowing.h>

window_t *curr_window = 0;
static int last_x, last_y;
static int dragging = 0;
static int down = 0;
extern context_t screen;
extern int need_cursor;
struct mouse_pos {
  int dx;
  int dy;
  int dz;
  int b;
};


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

volatile static int ready_counter = 4;
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
}

void EventsThread()
{
  struct message msg;
  char *buffer = malloc(sizeof(create_win_t) + MAX_TITLE_LEN);
  int buf_size = sizeof(create_win_t) + MAX_TITLE_LEN;

  resmgr_attach("/dev/pgs/main");
  ready_counter--;
  while (1) {
    msg.tid = _MSG_SENDER_ANY;
    msg.recv_buf = buffer;
    msg.flags = 0;		//MSG_MEM_SHARE;
    msg.recv_size = buf_size;
    alarm(50);
    receive(&msg);
    alarm(0);

    if (msg.tid != _MSG_SENDER_SIGNAL)
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
void mouse_thread()
{
  int oldb = 0;
  int psaux;

  do {
    psaux = open("/dev/psaux", 0);
    sched_yield();
  } while (!psaux);
  struct mouse_pos move;
  ready_counter--;
  while (1) {
    read(psaux, &move, sizeof(struct mouse_pos));
    if (move.dx || move.dy) {	// мышь сдвинули
      mousex += move.dx;
      mousey += move.dy;
      if (mousex < 0)
	mousex = 0;
      if (mousey < 0)
	mousey = 0;
      if (mousex > screen.w)
	mousex = screen.w;
      if (mousey > screen.h)
	mousey = screen.h;
      need_cursor = 1;
      int handle = get_window_handle(mousex, mousey);
      int win_x = 0, win_y = 0;
      window_t *win = NULL;
      if (handle) {
	win = GetWindowInfo(handle);
	if (!win)
	  continue;
	win_x = mousex - win->x;
	win_y = mousey - win->y;
	if (win->class & WC_MOUSEMOVE) {
	  if (win->class & WC_NODECORATIONS)
	    PostEvent(win->tid, handle, EV_MMOVE, win_x, win_y, 0, 0);
	  else {
	    if (win_x > 3 && win_x < win->w - 3 && win_y > 21 && win_y < win->h - 3)
	      PostEvent(win->tid, handle, EV_MMOVE, win_x - 3, win_y - 21, 0, 0);
	  }
	}
      }
      if (down) {
	if (handle && win) {
	  if (win->class & WC_NODECORATIONS)
	    continue;
	  if (!dragging) {
	    if ((win_x >= 3) && (win_y >= 3) && (win_y <= 18) && (win_y <= (win->w - 6))) {
	      curr_window = win;
	      last_x = mousex;
	      last_y = mousey;
	      SetFocusTo(handle);
	      dragging = 1;
	      win->x_drag = win->x;
	      win->y_drag = win->y;
	      DrawBorder(0);

	    }
	  }
	}
	if (dragging) {
	  if (curr_window) {
	    curr_window->x_drag += (mousex - last_x);
	    curr_window->y_drag += (mousey - last_y);
	    if (curr_window->x_drag < 0) {
	      mousex -= curr_window->x_drag;
	      curr_window->x_drag = 0;
	    }
	    if (curr_window->y_drag < 0) {
	      mousey -= curr_window->y_drag;
	      curr_window->y_drag = 0;
	    }
	    if (curr_window->x_drag + curr_window->w > screen.w) {
	      mousex += screen.w - (curr_window->x_drag + curr_window->w);
	      curr_window->x_drag = screen.w - curr_window->w;
	    }
	    if (curr_window->y_drag + curr_window->h > screen.h - 28) {
	      mousey += screen.h - (curr_window->y_drag + curr_window->h + 28);
	      curr_window->y_drag = screen.h - curr_window->h - 28;
	    }
	    last_x = mousex;
	    last_y = mousey;
	    DrawBorder(0);
	  }
	}

      }
 
    }
    if (move.b && !oldb) {	// кнопку нажали

      oldb = move.b;

      down = 1;
      need_cursor = 1;
      int handle = get_window_handle(mousex, mousey);

      if (!handle)
	continue;
      window_t *win = GetWindowInfo(handle);
      int win_x = mousex - win->x;
      int win_y = mousey - win->y;

      if (win->class & WC_NODECORATIONS) {
	PostEvent(win->tid, handle, EV_MDOWN, win_x, win_y, 0, 0);
      } else {
	if (win_x > 3 && win_x < win->w - 3 && win_y > 21 && win_y < win->h - 3)
	  PostEvent(win->tid, handle, EV_MDOWN, win_x - 3, win_y - 21, 0, 0);
      }

    }
    if (!move.b && oldb) {	// отпустили
      oldb = move.b;

    if (curr_window && dragging) {
      DrawBorder(-1);
      need_refresh = 1;
      need_cursor = 1;
      dragging = 0;
      curr_window = 0;
    }
    down = 0;
    int handle = get_window_handle(mousex, mousey);

    if (!handle)
     continue;
    window_t *win = GetWindowInfo(handle);
    int win_x = mousex - win->x;
    int win_y = mousey - win->y;

    if (win->class & WC_NODECORATIONS) {
      PostEvent(win->tid, handle, EV_MUP, win_x, win_y, 0, 0);
    } else {
      if ((win_x >= win->w - 21) && (win_y >= 5) && (win_x <= win->w - 5) && (win_y <= 19)) {
	PostEvent(win->tid, handle, EV_WINCLOSE, 0, 0, 0, 0);
	continue;
      }
      if (win_x > 3 && win_x < win->w - 3 && win_y > 21 && win_y < win->h - 3)
	PostEvent(win->tid, handle, EV_MUP, win_x - 3, win_y - 21, 0, 0);
    }
    if (!win->active) {
      SetFocusTo(handle);
      need_cursor = 1;
    }

    }
  }
}

void kb_thread()
{
  int kbd;

  do {
    kbd = open("/dev/keyboard", 0);
    sched_yield();
  } while (!kbd);
  char ch;
  int extended = 0;
  ready_counter--;
  while (1) {
    read(kbd, &ch, 1);
    if (ch == 0x01) {		// расширенный символ
      extended = 1;
      continue;
    }
    window_t *w = GetActiveWindow();

    if (w)
      PostEvent(w->tid, w->handle, EV_KEY, ch & 0xFF, extended, 0, 0);
    extended = 0;
  }
}


void MappingThread()
{
  resmgr_attach("/dev/pgs/mapping");
  struct message msg;
  ready_counter--;
  while (1) {
    msg.tid = _MSG_SENDER_ANY;
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
void StartEventHandling()
{
  thread_create((off_t) & mouse_thread);
  thread_create((off_t) & kb_thread);
  thread_create((off_t) & EventsThread);
  thread_create((off_t) & MappingThread);
  int iterations = 0;
  int lastuptime;
  int newuptime;
  lastuptime = newuptime = uptime();
  while(ready_counter) {
    newuptime = uptime();
    iterations += newuptime - lastuptime;
    lastuptime = newuptime;
    sched_yield();
  }
  printf("FGS: Event handling subsystem started up, elapsed time ~ %u ms\n", iterations);
}

