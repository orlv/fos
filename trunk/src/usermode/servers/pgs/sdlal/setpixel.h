/*
 * Portable Graphics System
 *  SDL abstraction layer
 * Copyright (c) 2007 Grindars
 */

#ifndef __SETPIXEL_H_
#define __SETPIXEL_H_

#include <gui/types.h>

void SetPixelInternal(int x, int y, int rgb_color, context_t * context);
void SetPixel(int x, int y, int color, context_t * context);

SDL_Surface *screen;

static inline int RGB2Pixel(int rgb)
{
  int R = (rgb >> 16) & 0xff;
  int G = (rgb >> 8) & 0xff;
  int B = rgb & 0xff;

  return SDL_MapRGB(screen->format, R, G, B);
}

#endif
