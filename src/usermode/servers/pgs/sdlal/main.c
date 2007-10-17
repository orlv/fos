/*
 * Portable Graphics System
 *  SDL abstraction layer
 * Copyright (c) 2007 Grindars
 */
// этот слой будет рассчитан на линукс !
#include <SDL.h>
#include <stdio.h>

#include <gui/types.h>

#include "events.h"

#define VIDEO_WIDTH 1024
#define VIDEO_HEIGHT 768

SDL_Surface *screen;
mode_definition_t graphics_init() {
	printf("Initialization of SDL layer\n");
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("FATAL: can't init SDL\n");
		exit(1);
	}
	atexit(SDL_Quit);

	SDL_ShowCursor(SDL_DISABLE);

	screen = SDL_SetVideoMode(VIDEO_WIDTH, VIDEO_HEIGHT, 16, SDL_SWSURFACE);
	if(screen == NULL) {
		printf("FATAL: can't set video mode\n");
		exit(1);
	}
	mode_definition_t mode = {VIDEO_WIDTH, VIDEO_HEIGHT, 2};
	return mode;
}
