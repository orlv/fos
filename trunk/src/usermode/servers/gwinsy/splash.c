/*
 * Copyright (C) 2008 Sergey Gridassov
 */

#include "config.h"
#if CONFIG_SPLASH == 0
int startup_splash() {
	return 0;
}

#else

#include <stdlib.h>
#include <string.h>
#include "context.h"
#include "picture.h"
#include "util.h"
#include "video.h"
#include "font.h"

static void print_centered_x(const char *str, u32_t color, int y, context_t *to) {
	int width = strlen(str) * 8;
	print(str, (screen->w - width) / 2, y, color, to);
}

int startup_splash() {
	picture_t *fos_logo = load_file("/usr/share/pixmaps/fos-logo.pct");

	if(fos_logo == NULL)
		return -1;

	picture_t *gwinsy_logo = load_file("/usr/share/pixmaps/gwinsy-logo.pct");

	if(gwinsy_logo == NULL)
		return -1;

	draw_picture(fos_logo, 10, 10, screen);
	
	draw_picture(gwinsy_logo, (screen->w - gwinsy_logo->width) / 2, screen->h  / 2 - gwinsy_logo->height, screen);

	print_centered_x("Starting up", 0xFFFFFF, screen->h / 2 + 16, screen);

	free(fos_logo);
	free(gwinsy_logo);
	return 0;
}

#endif
