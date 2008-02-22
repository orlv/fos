/*
 * Copyright (C) 2007 - 2008 Sergey Gridassov
 * TODO:
 * - Если кнопки задач не влезают - что-нибудь типа стрелок
 */

#include <stdio.h>
#include <string.h>
#include <fgs/fgs.h>
#include <fgs/controls.h>
#include <fos/fos.h>
#include <stdlib.h>
#include <sys/rtc.h>
#include <fos/message.h>

#define ITEMS_COUNT 4
static char *items[ITEMS_COUNT] = { "FTerm", "Tetris", "Controls test", "Drawing speed test"};
static char *cmds[ITEMS_COUNT] = { "/usr/bin/fterm", "/usr/bin/tetris", "/usr/bin/test", "/usr/bin/drawtest" };

int width, height;
int hndl;
int displayed = 0;
int menu;
int buttonscount = 0;
int *buttons = NULL;
int clock;
int wincount = 0;
int startbutton;

typedef struct twin {
	struct twin *next;
	int handle;
	char *title;
	int button;
} taskbar_window_t;

taskbar_window_t *windows = NULL;

void RedrawTaskBar() {
  for(int i = 0; i < buttonscount; i++) {
    DestroyControl(buttons[i]);
  }
  free(buttons);
  buttonscount = wincount;
  if(wincount) {
    int butpanel = width - 55 - 5 - 60;
    int onebutton = butpanel / wincount;
    if(onebutton > butpanel / 2) onebutton = butpanel / 2;
    buttons = malloc(buttonscount * 4);
    taskbar_window_t *ptr = windows;
    for(int i = 0; i < buttonscount; i++, ptr = ptr->next) {
      if(strlen(ptr->title) *8 > onebutton - 16) {
        int chars = (onebutton - 16) / 8;
        char *buf = malloc(chars + 1);
        strncpy(buf, ptr->title, chars -3);
        strcat(buf, "...");
        buttons[i] = CreateButton(hndl, 55 + onebutton * i, 3, onebutton - 2, 22, buf);
        free(buf);
      } else
        buttons[i] = CreateButton(hndl, 55 + onebutton * i, 3, onebutton - 2, 22, ptr->title);
      ptr->button = buttons[i];
    }
  }
//  RefreshWindow(drawing);
}

int EventHandler(int hwnd, int class, int a0, int a1, int a2, int a3)
{
  switch (class) {
  case EV_WINCLOSE:
    DestroyControlsWindow(hwnd);
    return 0;
  case EVC_CLICK:
    if(hwnd == startbutton) {
      if (!displayed) {
	menu = CreateMenu(hndl, 0, height - 28, ITEMS_COUNT, items);	// hndl, 3
	displayed = 1;
      } else {
	DestroyControl(menu);
	displayed = 0;
      }
      return 1;
    }
    for(taskbar_window_t *ptr = windows; ptr; ptr = ptr->next) {
      if(hwnd == ptr->button) {
        SetFocus(ptr->handle);
        break;
      }
    }
    break;
  case EVC_MENU:
    DestroyControl(hwnd);
    displayed = 0;
    exec(cmds[a0], NULL);
    return 1;
  case EV_NEWWIN: {
    taskbar_window_t *new = malloc(sizeof(taskbar_window_t));
    new->next = windows;
    new->handle = a0;
    new->title = malloc(MAX_TITLE_LEN);
    GetWindowTitle(new->handle, new->title);
    windows = new;
    wincount++;
    RedrawTaskBar();
    break;
  }
  case EV_DESTROYWIN:
     for(taskbar_window_t *ptr = windows, *prev = NULL; ptr; prev = ptr, ptr = ptr->next) {
       if(ptr->handle == a0) {
         if(prev) 
           prev->next = ptr->next;
         else
           windows = ptr->next;
         free(ptr->title);
         free(ptr);
       }
    }
    wincount--;
    RedrawTaskBar();
    break;  
  }
  return 1;

}

THREAD(clock_thread) {
	struct time time;
	char tmp[8];
	int oldmin = -1;
	while(1) {
		if(get_time(&time) < 0)
			SetControlText(clock, "RTC?");
		else if(oldmin != time.min) {
			snprintf(tmp, 8, "%02u:%02u", time.hour, time.min);
			SetControlText(clock, tmp);
		}
		oldmin = time.min;
		alarm(1000);
		struct message msg;
		msg.recv_size = 0;
		msg.send_size = 0;
		msg.flags = MSG_ASYNC;
		msg.tid = 0;
		receive(&msg);

	}
}

asmlinkage int main(int argc, char **argv)
{
  GUIInit();
  ScreenInfo(&width, &height);
  hndl = CreateControlsWindow(0, height - 28, width, 28, "TaskBar", EventHandler, WC_NODECORATIONS | WC_WINDOWSEVENTS);
  int drawing = GetDrawingHandle(hndl);

  line(drawing, 0, 0, width, 0, 0xD8D8D8);
  line(drawing, 0, 1, width, 1, 0xF8F8F8);
  line(drawing, width - 57, 4, width - 4, 4, 0x787878);
  line(drawing, width - 57, 4, width - 57, 24, 0x787878);
  line(drawing, width - 57, 25, width - 3, 25, 0xFFFFFF);
  line(drawing, width - 3, 4, width - 3, 25, 0xFFFFFF);
  startbutton = CreateButton(hndl, 3, 4, 48, 22, "Menu");
  clock = CreateStatic(hndl, width - 50, 7, 40, 16, "RTC?");
  ControlsWindowVisible(hndl, 1);
//  thread_create((off_t) clock_thread);
  ControlsMessageLoop();
  GuiEnd();
  return 0;
}
