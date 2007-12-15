/*
 * Portable Graphics System
 *  SDL abstraction layer
 * Copyright (c) 2007 Grindars
 */

#include <SDL.h>
#include "setpixel.h"
#include "font.h"

SDL_Surface *screen;

void PutString(int x, int y, char *str, int color, context_t * context)
{
  int origx = x;

  if (SDL_MUSTLOCK(screen) && context == NULL)
    SDL_LockSurface(screen);
  for (; *str; str++) {
    for (int i = 0; i < 16; i++)
      for (int j = 0; j < 8; j++)
	if (font[16 * *str + i] & (1 << j))
	  SetPixelInternal(x + 8 - j, y + i, color, context);
    x += 8;
  }
  if (context == NULL) {
    if (SDL_MUSTLOCK(screen))
      SDL_UnlockSurface(screen);
    SDL_UpdateRect(screen, origx, y, x - origx, 16);
  }
}
