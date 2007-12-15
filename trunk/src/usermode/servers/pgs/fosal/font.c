/*
 * Portable Graphics System
 *  SDL abstraction layer
 * Copyright (c) 2007 Grindars
 */

#include <gui/al.h>
#include <stddef.h>
//#include "font.h"

char *font = NULL;

void PutString(int x, int y, char *str, int color, context_t * context)
{
  if (!font)
    font = load_file("/usr/share/fonts/font.psf");

  for (; *str; str++) {
    for (int i = 0; i < 16; i++)
      for (int j = 0; j < 8; j++)
	if (font[16 * (unsigned char)*str + i + 4] & (1 << j))
	  SetPixel(x + 8 - j, y + i, color, context);
    x += 8;
  }
}
