/*
 * Copyright (C) 2008 Sergey Gridassov
 */

#ifndef PICTURE_H
#define PICTURE_H

#include <types.h>

typedef struct {
	char sig[4];
	u32_t width;
	u32_t height;
	u32_t keycolor;
	u32_t data[];
} __attribute__((__packed__)) picture_t;

int draw_picture(picture_t *pict, int x, int y, context_t *to);
#endif
