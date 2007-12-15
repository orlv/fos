/*
 * Portable Graphics System
 *  FOS abstraction layer
 * Copyright (c) 2007 Grindars
 */

#include <gui/types.h>
#include <gui/al.h>

void DrawImage(int x, int y, picture_t * pict, context_t * context)
{
  for (int cy = 0; cy < pict->height; cy++)
    for (int cx = 0; cx < pict->width; cx++) {
      int pixel = pict->data[cy * pict->width + cx];

      if (pixel == pict->keycolor)
	continue;		// прозрачный пиксель
      SetPixel(x + cx, y + cy, pixel, context);
    }
}
