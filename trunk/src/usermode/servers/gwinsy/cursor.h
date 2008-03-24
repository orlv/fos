/*
 * Copyright (C) 2008 Sergey Gridassov
 */

#ifndef CURSOR_H
#define CURSOR_H

#define CURSOR_POINTER	0

void cursor_init(void);
void cursor_move(int x, int y);
void cursor_sync();
int cursor_select(unsigned int type);
#endif
