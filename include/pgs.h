/*
 * include/pgs.h
 * Функции GUI
 */
 
#ifndef _PGS_H
#define _PGS_H
void GUIInit();
int CreateWindow(int w, int h, char *caption, int flags);
void WaitEvent(int *class, int *handle, int *a0, int *a1, int *a2, int *a3) ;
void DestroyWindow(int handle) ;
void GuiEnd();
void pixel(int handle, int x, int y, int color);
void RefreshWindow(int handle);
void rect(int handle, int x, int y, int width, int height, int color);
void line(int handle, int x0, int y0, int x1, int y1, int color);
void pstring(int handle, int x, int y, int color, char *str);

#define WC_WINDOW 0
#define EV_WINCLOSE 1
#define EV_MDOWN 2
#define EV_MUP 3
#define EV_KEY 4
#endif
