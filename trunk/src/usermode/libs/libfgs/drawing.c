/*
  Copyright (C) 2007 Serge Gridassov
 */

#include <fgs/fgs.h>
#include "privatetypes.h"

void Draw3D(int x, int y, int w, int h, int handle, int style)
{
  switch (style) {
  case STYLE_BUTTON_NORMAL:
    rect(handle, x, y, w, h, 0xc3c3c3);
    line(handle, x + 1, y + 1, x + 1, y + h - 3, 0xffffff);
    line(handle, x + 1, y + 1, x + w - 3, y + 1, 0xffffff);
    line(handle, x + 1, y + h - 2, x + w - 2, y + h - 2, 0x828282);
    line(handle, x + w - 2, y + h - 1, x + w - 2, y + 1, 0x828282);
    line(handle, x + 0, y + h - 1, x + w - 1, y + h - 1, 0x000000);
    line(handle, x + w - 1, y + h - 1, x + w - 1, y + 0, 0x000000);
    break;
  case STYLE_BUTTON_DOWN:
    rect(handle, x, y, w, h, 0xc3c3c3);
    line(handle, x, y, x + w - 1, y, 0);
    line(handle, x, y + h - 1, x + w - 1, y + h - 1, 0);
    line(handle, x, y, x, y + h - 1, 0);
    line(handle, x + w - 1, y, x + w - 1, y + h - 1, 0);
    line(handle, x + 1, y + 1, x + w - 2, y + 1, 0x787878);
    line(handle, x + 1, y + h - 2, x + w - 2, y + h - 2, 0x787878);
    line(handle, x + 1, y + 1, x + 1, y + h - 2, 0x787878);
    line(handle, x + w - 2, y + 1, x + w - 2, y + h - 2, 0x787878);
    break;
  }
}
