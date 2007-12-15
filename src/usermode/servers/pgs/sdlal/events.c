/*
 * Portable Graphics System
 *  SDL abstraction layer
 * Copyright (c) 2007 Grindars
 */

// этот слой будет рассчитан на линукс !

#include <SDL.h>
#include <gui/types.h>

void event_handler(event_t * event);

int event_thread(void *reserved)
{
  SDL_Event event;

  while (1) {
    SDL_WaitEvent(&event);
    switch (event.type) {
    case SDL_MOUSEMOTION:{
	mousemove_event_t mouse = { event.motion.x, event.motion.y };
	event_t event = { EVENT_TYPE_MOUSEMOVE, &mouse };
	event_handler(&event);
	break;
      }
    case SDL_QUIT:
      printf("SDLAL: user requested exit\n");
      exit(0);
      break;
    case SDL_MOUSEBUTTONDOWN:{
	event_t event = { EVENT_TYPE_MOUSEDOWN, NULL };
	event_handler(&event);
	break;
      }
    case SDL_MOUSEBUTTONUP:{
	event_t event = { EVENT_TYPE_MOUSEUP, NULL };
	event_handler(&event);
	break;
      }
    }
  }
  return 0;
}

void StartEventHandling()
{
  SDL_CreateThread(event_thread, NULL);
}
