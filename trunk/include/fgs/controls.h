/*
 * include/controls.h
 */
#ifndef CONTROLS_H
#define CONTROLS_H
#if defined(__cplusplus)
extern "C" {
#endif
int CreateControlsWindow(int x, int y, int w, int h, char *title,  int (*handler)(int, int, int, int, int, int), int style);
void DestroyControlsWindow(int hndl);
void ControlsMessageLoop();
int CreateButton(int window, int x, int y, int w, int h, char *caption);
void DestroyControl(int handle);
void ControlsWindowVisible(int handle, int visible);
int CreateStatic(int window, int x, int y, int w, int h, char *caption);
void SetControlText(int handle, char *text);
int GetDrawingHandle(int handle);
int CreateMenu(int hndl, int x, int y, int count, char *items[]);
void MoveFocusToControl(int win, int handle);
#define EVC_CLICK	0xF0000001
#define EVC_MENU	0xF0000002
#if defined(__cplusplus)
}
#endif
#endif
