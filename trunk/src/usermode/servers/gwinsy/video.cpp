/*
 * Copyright (C) 2008 Sergey Gridassov
 */

#include <stdio.h>
#include <string.h>
#include "context.h"
#include "video.h"
#include "config.h"

int vesa_init(int width, int height, int bpp, context_t **screen, jump_table_t **jmptbl);

static const struct {
	const char *driver;
	int (*init)(int width, int height, int bpp, context_t **screen, jump_table_t **jmptbl);
} drivers[] = {
#if CONFIG_VESA == 1
	{ "vesa", vesa_init }, 
#endif
};

int video_init(const char *driver, int width, int height, int bpp, context_t **screen, jump_table_t **jmptbl) {
	for(unsigned int i = 0; i < sizeof(drivers) / sizeof(drivers[0]); i++)
		if(!strcmp(driver, drivers[i].driver)) 
			return (drivers[i].init)(width, height, bpp, screen, jmptbl);
	
	printf("Error: driver '%s' not supported\n", driver);
	return -1;
}
