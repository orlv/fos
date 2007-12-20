/*
 *         FOS Graphics System
 * Copyright (c) 2007 Sergey Gridassov
 */

#include <private/types.h>

void border(int x, int y, int w, int h, context_t * context)
{
  unsigned short *ptr;

  w--;
  h--;
  int pitch = context->w;

  ptr = (unsigned short *)context->data + x + y * pitch;
  for (int i = 0; i < w; i++) {
    if (i & 1)
      continue;
    ptr[i] ^= 0xFFFF;
    ptr[h * pitch + i] ^= 0xFFFF;
  }
  for (int i = 0; i < h; i++) {
    if (i & 1)
      continue;
    ptr[i * pitch] ^= 0xFFFF;
    ptr[i * pitch + w] ^= 0xFFFF;
  }
}
