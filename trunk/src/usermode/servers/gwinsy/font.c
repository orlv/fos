/*
 * Copyright (C) 2008 Sergey Gridassov
 */

#include <types.h>

#include "context.h"
#include "font.h"
#include "video.h"
#include "util.h"

static u8_t *font;

void print(const char *str, int x, int y, u32_t color, context_t *context) {
  if(font == NULL) 
    font = load_file("/usr/share/fonts/8x16.psf");
  
  for (; *str; str++) {
    for (int i = 0; i < 16; i++)
      for (int j = 0; j < 8; j++)
	if (font[16 * (unsigned char)*str + i + 4] & (1 << j))
	  SetPixel(x + 8 - j, y + i, color, context);
    x += 8;
  }
}
