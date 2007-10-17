/*
 * Portable Graphics System
 *       GUI system
 * Copyright (c) 2007 Grindars
 */
#include <gui/al.h>
#include <stddef.h>

extern context_t *backbuf;

int mousex;
int mousey;
int cursorx = 0;
int cursory = 0;
#include "cursormap.h"
#include "windowing.h"
void RedrawCursor() {
	FlushContext(backbuf, cursor.width, cursor.height, cursorx, cursory, cursorx, cursory , NULL);
	DrawImage(mousex, mousey, &cursor, NULL);
	cursorx = mousex;
	cursory = mousey;
}

