/*
 * Portable Graphics System
 *       GUI system
 * Copyright (c) 2007 Grindars
 */
// общие структуры
#ifndef GUI_TYPES_H
#define GUI_TYPES_H
/*
typedef struct {
	int width;
	int height;
	int bpp; // bytes per pixel
} mode_definition_t;
*/
#define EVENT_TYPE_MOUSEMOVE 1
#define EVENT_TYPE_MOUSEDOWN 2
#define EVENT_TYPE_MOUSEUP 3

typedef struct {
	int x;
	int y;
} mousemove_event_t;
typedef struct {
	int type;
	mousemove_event_t *mousemove;
} event_t;

typedef struct {
	int width;
	int height;
	int keycolor;
	int data[];
} picture_t;

typedef struct {
	int w;
	int h;
	int bpp;
	int native_pixels;
	char *data;
} context_t;
typedef struct {
	int w;
	int h;
	int class;
} create_win_t;
struct win_info {
	int bpp;
	int handle;
	int margin_up;
	int margin_down;
	int margin_left;
	int margin_right;
};
#define MAX_TITLE_LEN 64
#define WIN_CMD_CREATEWINDOW 1
#define WIN_CMD_DESTROYWINDOW 2
#define WIN_CMD_WAIT_EVENT 3
#define WIN_CMD_CLEANUP 4
#define WIN_CMD_MAPBUF 5
#define WIN_CMD_REFRESHWINDOW 6
#define EV_WINCLOSE 1
#define EV_MDOWN 2
#define EV_MUP 3
#endif
