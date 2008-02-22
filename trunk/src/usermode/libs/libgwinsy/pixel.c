/*
  Copyright (C) 2007 Serge Gridassov
 */

#include "privatetypes.h"

void pixel(int handle, int x, int y, int color)
{
  struct win_hndl *wh = (struct win_hndl *)handle;

  if (x > wh->w || y > wh->h|| y < 0 || x < 0)
    return;

  x += wh->margin_left;
  y += wh->margin_up;
  unsigned short *ptr = (unsigned short *)wh->data;

  ptr[x + y * (wh->w + wh->margin_left + wh->margin_right)] =
      RED(color, 5) << 11 | GREEN(color, 6) << 5 | BLUE(color, 5);
}
