/*
  Copyright (C) 2007 Serge Gridassov
 */

#include "privatetypes.h"

void rect(int handle, int x, int y, int width, int height, int color)
{
  struct win_hndl *wh = (struct win_hndl *)handle;

  x += wh->margin_left;
  y += wh->margin_up;

  unsigned short modecolor = RED(color, 5) << 11 | GREEN(color, 6) << 5 | BLUE(color, 5);

  int y_limit = height + y;

  for (; y < y_limit; y++) {
    unsigned short *dot = (unsigned short *)wh->data + y * (wh->w + wh->margin_left + wh->margin_right) + x;	// * context->w

    for (int xx = 0; xx < width; xx++)
      *(dot++) = modecolor;
  }
}
