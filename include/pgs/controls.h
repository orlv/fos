/*
 * include/controls.h
 */
#ifndef CONTROLS_H
#define CONTROLS_H
int CreateControlsWindow(int w, int h, char *title, int (*handler)(int, int, int, int, int, int));
void DestroyControlsWindow(int hndl);
void ControlsMessageLoop();
int CreateButton(int window, int x, int y, int w, int h, char *caption);
void DestroyControl(int handle);
void ControlsWindowVisible(int handle, int visible);
int CreateStatic(int window, int x, int y, int w, int h, char *caption);
void SetControlText(int handle, char *text);
#define EVC_CLICK 0xF0000001
#endif
