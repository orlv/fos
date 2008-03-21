/*
 * Copyright (C) 2008 Sergey Gridassov
 */

#include "context.h"
#include "picture.h"
#include "video.h"

int draw_picture(picture_t *pict, int x, int y, context_t *to) {
	for(int yy = 0; yy < pict->height; yy++)
		for(int xx = 0; xx < pict->width; xx++) {
			u32_t rgb = pict->data[yy * pict->width + xx];
			if(rgb == pict->keycolor)
				continue;
			SetPixel(xx + x , yy + y, rgb, to);
		}

	return 0;
}
