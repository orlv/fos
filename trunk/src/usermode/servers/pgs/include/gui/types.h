/*
 * Portable Graphics System
 *       GUI system
 * Copyright (c) 2007 Grindars
 */
// общие структуры
#ifndef GUI_TYPES_H
#define GUI_TYPES_H
typedef struct {
	int width;
	int height;
	int bpp; // bytes per pixel
} mode_definition_t;

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
#endif