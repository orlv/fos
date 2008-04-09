/*
 * Copyright (C) 2008 Sergey Gridassov
 */

#include "context.h"
#include "video.h"
#include "assert.h"
#include "cursor.h"
#include "windows.h"
#include "picture.h"
#include "util.h"
#include "ipc.h"

typedef struct {
	picture_t *pict;
	const char *filename;
	int hot_x;
	int hot_y;
} cursor_t;

static cursor_t cursor_table[] = {
/* CURSOR_POINTER */ { NULL, "/usr/share/cursors/pointer.pct", 0, 0 },
};

static cursor_t *current = NULL;

static int old_x, old_y, cur_x, cur_y;


int cursor_select(unsigned int type) {
	if(type >= sizeof(cursor_table) / sizeof(cursor_t))
		return -1;

	current = &cursor_table[type];
	return 0;
}

void cursor_move(int x, int y) {
	old_x = cur_x;
	old_y = cur_y;
	cur_x = x;
	cur_y = y;

	int dx = cur_x - old_x;	// смысл в том, чтобы в событии
	int dy = cur_y - old_y;	// d[xy] не выходили за экран

	PostEvent(0, 0, EV_GLOBAL, EVG_MMOVE, cur_x, cur_y, dx, dy);
}

void cursor_sync() {
//	windows_handle_move(cur_x, cur_y);

	Blit(backbuf, screen, old_x - current->hot_x, old_y - current->hot_y, current->pict->width, current->pict->height, old_x - current->hot_x, old_y - current->hot_y);
	draw_picture(current->pict, cur_x - current->hot_x, cur_y - current->hot_y, screen);
}

void cursor_init(void) {
	for(unsigned int i = 0; i < sizeof(cursor_table) / sizeof(cursor_t); i++) {
		cursor_table[i].pict = (picture_t *) load_file(cursor_table[i].filename);
		assert(cursor_table[i].pict != 0);
		
		cursor_table[i].hot_x = cursor_table[i].pict->hot_x;
		cursor_table[i].hot_y = cursor_table[i].pict->hot_y;
	}
	cursor_move(screen->w / 2, screen->h / 2);
	cursor_select(CURSOR_POINTER);
}

void cursor_shift(int dx, int dy) {
	old_x = cur_x;
	old_y = cur_y;
	cur_x+= dx;
	cur_y+= dy;
	
	if(cur_x < 0)
		cur_x = 0;

	if(cur_y < 0)
		cur_y = 0;

	if(cur_x > screen->w)
		cur_x = screen->w;

	if(cur_y > screen->h)
		cur_y = screen->h;

	dx = cur_x - old_x;	// смысл в том, чтобы в событии
	dy = cur_y - old_y;	// d[xy] не выходили за экран

	PostEvent(0, 0, EV_GLOBAL, EVG_MMOVE, cur_x, cur_y, dx, dy);
}
