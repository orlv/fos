/*
 *         FOS Graphics System
 * Copyright (c) 2007 Sergey Gridassov
 */

#include <private/types.h>
#include <stddef.h>

#define RED(x, bits)	((x >> (16 + 8 - bits)) & ((1 << bits) - 1))
#define GREEN(x, bits)	((x >> (8 + 8 - bits)) & ((1 << bits) - 1))
#define BLUE(x, bits)	((x >> (8 - bits)) & ((1 << bits) - 1))

void SetPixel(int x, int y, int rgb, context_t * context)
{
  if (x < 0 || y < 0 || x >= context->w || y >= context->h)
    return;
  unsigned short *ptr = (unsigned short *)context->data;

  if (context->native_pixels)
    ptr[x + y * context->w] = rgb;
  else
    ptr[x + y * context->w] = RED(rgb, 5) << 11 | GREEN(rgb, 6) << 5 | BLUE(rgb, 5);
}

int GetPixel(int x, int y, context_t * context)
{
  int color;
  unsigned short *ptr = (unsigned short *)context->data;

  color = ptr[x + y * context->w];
  if (context->native_pixels)
    return color;
  else
    return (((color >> 11) & 0x1f) << (16 + 3)) | (((color >> 5) & 0x3f) << (8 + 2)) | ((color & 0x1f) << 3);
}
