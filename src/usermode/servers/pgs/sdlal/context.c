/*
 * Portable Graphics System
 *  SDL abstraction layer
 * Copyright (c) 2007 Grindars
 */
#include <SDL.h>
#include <gui/types.h>
SDL_Surface *screen; 
void FlushBackBuffer(char *back) {
	if (SDL_MUSTLOCK(screen)) 
		SDL_LockSurface(screen);
	memcpy(screen->pixels, back, screen->pitch  * screen->h);
	if (SDL_MUSTLOCK(screen)) 
		SDL_UnlockSurface(screen);
	SDL_UpdateRect(screen, 0, 0, screen->w, screen->h);
}
void FlushContext(context_t  *context, int w, int h, int x, int y, int srcx, int srcy, context_t *target) {
	SDL_Surface *surf , *trg;
	SDL_Rect src = {srcx, srcy, w +1, h +1};
	SDL_Rect dst = {x, y, w + 1, h +1};
	surf = SDL_CreateRGBSurfaceFrom(context->data, context->w, context->h, context->bpp * 8, context->w * context->bpp, 0, 0, 0, 0);
	if(target == NULL) {
		SDL_BlitSurface(surf, &src, screen, &dst);
		SDL_FreeSurface(surf);
		SDL_UpdateRect(screen, x, y, w + 1, h +1);
	}else {
		trg = SDL_CreateRGBSurfaceFrom(target->data, target->w, target->h, target->bpp * 8, target->w * context->bpp, 0, 0, 0, 0);
		SDL_BlitSurface(surf, &src, trg, &dst);
		SDL_UpdateRect(trg, x, y, w + 1, h +1);
		SDL_FreeSurface(surf);
		SDL_FreeSurface(trg);
	}

}
