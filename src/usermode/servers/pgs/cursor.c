/*
 * Portable Graphics System
 *       GUI system
 * Copyright (c) 2007 Grindars
 */

#include <gui/al.h>
#include <stddef.h>
//#include "cursormap.h"
#include "windowing.h"

extern context_t *backbuf;
extern context_t screen;
extern picture_t *busy, *cursor;
int mousex;
int mousey;
int cursorx = 0;
int cursory = 0;

picture_t *cur = NULL;
static int busylevel = 1;
extern int need_cursor;

void SetBusy(int state)
{
  busylevel += state;
  if (busylevel < 0)
    busylevel = 0;
  if (busylevel == 0)
    cur = cursor;
  else
    cur = busy;
  need_cursor = 1;
}

void RedrawCursor()
{
  if (!cur)
    cur = busy;
  FlushContext(backbuf, cur->width, cur->height, cursorx, cursory, cursorx, cursory, &screen);
  DrawImage(mousex, mousey, cur, &screen);
  cursorx = mousex;
  cursory = mousey;
}
