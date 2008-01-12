/*
  Copyright (C) 2007 Serge Gridassov
 */

#include <stdio.h>
#include <string.h>
#include <fgs/fgs.h>
#include <fgs/controls.h>
#include <fos/fos.h>
#include <stdlib.h>

#define ITEMS_COUNT 3
static char *items[ITEMS_COUNT] = { "FTerm", "Tetris", "Controls test" };
static char *cmds[ITEMS_COUNT] = { "/usr/bin/fterm", "/usr/bin/tetris", "/usr/bin/test" };
int width, height;
int hndl;
int displayed = 0;
int menu;
int buttonscount = 0;
int *buttons = NULL;;
typedef struct twin {
	struct twin *next;
	int handle;
	char *title;
	int button;
} taskbar_window_t;
taskbar_window_t *windows = NULL;
int wincount = 0;
int startbutton;
void RedrawTaskBar() {
  int drawing = GetDrawingHandle(hndl);
  for(int i = 0; i < buttonscount; i++) {
    DestroyControl(buttons[i]);
  }
  free(buttons);
  buttonscount = wincount;
  if(wincount) {
    int onebutton = (width - 55 - 5) / wincount;
    if(onebutton > 162) onebutton = 162;
    buttons = malloc(buttonscount * 4);
    taskbar_window_t *ptr = windows;
    for(int i = 0; i < buttonscount; i++, ptr = ptr->next) {
      buttons[i] = CreateButton(hndl, 55 + onebutton * i, 3, onebutton - 2, 22, ptr->title);
      ptr->button = buttons[i];
    }
  }
  RefreshWindow(drawing);
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

asmlinkage int main(int argc, char **argv)
{
  GUIInit();
  ScreenInfo(&width, &height);
  hndl = CreateControlsWindow(0, height - 28, width, 28, "TaskBar", EventHandler, WC_NODECORATIONS | WC_WINDOWSEVENTS);
  int drawing = GetDrawingHandle(hndl);

  line(drawing, 0, 0, width, 0, 0xD8D8D8);
  line(drawing, 0, 1, width, 1, 0xF8F8F8);

  startbutton = CreateButton(hndl, 3, 4, 48, 22, "Menu");
  ControlsWindowVisible(hndl, 1);
  ControlsMessageLoop();
  GuiEnd();
  return 0;
}
