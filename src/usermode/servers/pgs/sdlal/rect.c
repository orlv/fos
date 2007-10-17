/*
 * Portable Graphics System
 *  SDL abstraction layer
 * Copyright (c) 2007 Grindars
 */
#include <SDL.h>

#include "setpixel.h"

 void DrawRect(int x, int y, int width, int height, int color, context_t *context)
{
	int modecolor;
	if(context != NULL) {if(context->native_pixels) 
		modecolor = color;
	else
		modecolor = RGB2Pixel(color);
	}

	int y_limit = height + y;
	if(context == NULL) {
		if (SDL_MUSTLOCK(screen)) 
			SDL_LockSurface(screen); 
		for(; y < y_limit; y++) {
			Uint16 *dot = (Uint16 *)screen->pixels + y*screen->w + x;
			for(int xx = 0; xx < width; xx++) 
				*(dot++) = modecolor;
		}
		if (SDL_MUSTLOCK(screen)) 
			SDL_UnlockSurface(screen); 
		SDL_UpdateRect(screen, x, y, width, height); 
	} else {
			for(; y < y_limit; y++) {
			Uint16 *dot = (Uint16 *)context->data + y*context->w + x;
			for(int xx = 0; xx < width; xx++) 
				*(dot++) = modecolor;
		}
	}
}
