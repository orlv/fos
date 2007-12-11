/*
 * include/pgs.h
 * Функции GUI
 */
 
#ifndef _PGS_H
#define _PGS_H
void GUIInit();
int CreateWindow(int x, int y, int w, int h, char *caption, int flags, int *evhndl);
void WaitEvent(int *class, int *handle, int *a0, int *a1, int *a2, int *a3) ;
void DestroyWindow(int handle) ;
void GuiEnd();
void pixel(int handle, int x, int y, int color);
void RefreshWindow(int handle);
void rect(int handle, int x, int y, int width, int height, int color);
void line(int handle, int x0, int y0, int x1, int y1, int color);
void pstring(int handle, int x, int y, int color, char *str);
void SetVisible(int handle, int visible);
void ScreenInfo(int *width, int *height);
#define WC_WINDOW 0
#define WC_NODECORATIONS	1
#define EV_WINCLOSE 1
#define EV_MDOWN 2
#define EV_MUP 3
#define EV_KEY 4
#endif
