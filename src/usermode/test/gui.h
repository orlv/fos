#ifndef __GUI_H
#define __GUI_H
void GUIInit();
int CreateWindow(int parent, int x, int y, int w, int h, char *caption, int flags);
void WaitEvent(int *class, int *handle, int *a0, int *a1, int *a2, int *a3) ;
void DestroyWindow(int handle) ;
void GuiEnd();
#define WC_WINDOW 0
#define WIN_CMD_CREATEWINDOW 1
#define WIN_CMD_DESTROYWINDOW 2
#define WIN_CMD_WAIT_EVENT 3
#define WIN_CMD_CLEANUP 4
#define WIN_CMD_MAPBUF 5
#define EV_WINCLOSE 1
#define EV_MDOWN 2
#define EV_MUP 3
#endif
