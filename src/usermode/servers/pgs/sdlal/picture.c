/*
 * Portable Graphics System
 *  SDL abstraction layer
 * Copyright (c) 2007 Grindars
 */
#include <SDL.h>

#include <gui/types.h>

#include "setpixel.h"

SDL_Surface *screen;
void DrawImage(int x, int y, picture_t *pict, context_t *context) {
	if (SDL_MUSTLOCK(screen) && context == NULL) 
		SDL_LockSurface(screen);
	for(int cy = 0; cy < pict->height; cy ++) 
	for(int cx = 0; cx < pict->width; cx ++) {
		int pixel = pict->data[cy * pict->width + cx];
		if(pixel == pict->keycolor) continue; // прозрачный пиксель
		SetPixelInternal(x + cx, y + cy, pixel, context);
	}
	if(context == NULL) {
		if (SDL_MUSTLOCK(screen)) 
			SDL_UnlockSurface(screen);
		SDL_UpdateRect(screen, x, y, pict->width, pict->height);
	}

}
