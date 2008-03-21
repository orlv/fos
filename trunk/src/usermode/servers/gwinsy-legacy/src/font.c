/*
 *         FOS Graphics System
 * Copyright (c) 2007 Sergey Gridassov
 */
#include <private/pixel.h>
#include <stddef.h>

char *font = NULL;

void PutString(int x, int y, char *str, int color, context_t * context)
{
  for (; *str; str++) {
    for (int i = 0; i < 16; i++)
      for (int j = 0; j < 8; j++)
	if (font[16 * (unsigned char)*str + i + 4] & (1 << j))
	  SetPixel(x + 8 - j, y + i, color, context);
    x += 8;
  }
}
