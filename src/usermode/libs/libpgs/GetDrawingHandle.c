/*
  Copyright (C) 2007 Serge Gridassov
 */

#include "privatetypes.h"

int GetDrawingHandle(int handle)
{
  rootwindow_t *rw = (rootwindow_t *) handle;
  return rw->handle;
}
