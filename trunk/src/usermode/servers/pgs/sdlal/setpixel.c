/*
 * Portable Graphics System
 *  SDL abstraction layer
 * Copyright (c) 2007 Grindars
 */

#include <SDL.h>
#include <gui/types.h>
#include "setpixel.h"

SDL_Surface *screen;

/* эта функция _не_ блокирует экран
  _не_ сообщает об обновлениях! */
int GetPixel(int x, int y, context_t * context)
{
  int color;
  Uint16 *bufp = (Uint16 *) context->data + y * context->w * context->bpp / 2 + x;

  color = *bufp;
  Uint8 R, G, B;

  if (!context->native_pixels) {
    SDL_GetRGB(color, screen->format, &R, &G, &B);
    return R << 16 | G << 8 | B;
  } else
    return color;
}

void SetPixelInternal(int x, int y, int rgb_color, context_t * context)
{
  if (x >= screen->w || y >= screen->h)
    return;			/* мы же не хотим ошибку сегментации? */
  int color;

  if (context == NULL) {
    color = RGB2Pixel(rgb_color);
    Uint16 *bufp = (Uint16 *) screen->pixels + y * screen->w + x;

    *bufp = color;
    return;
  }
  if (context->native_pixels)
    color = rgb_color;
  else
    color = RGB2Pixel(rgb_color);
  Uint16 *bufp = (Uint16 *) context->data + y * context->w + x;

  *bufp = color;

}

void SetPixel(int x, int y, int color, context_t * context)
{
  if (SDL_MUSTLOCK(screen) && context == NULL)
    SDL_LockSurface(screen);
  SetPixelInternal(x, y, color, context);
  if (context == NULL) {
    if (SDL_MUSTLOCK(screen))
      SDL_UnlockSurface(screen);
    SDL_UpdateRect(screen, x, y, 1, 1);
  }
}
