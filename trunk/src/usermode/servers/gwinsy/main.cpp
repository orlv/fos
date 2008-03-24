/*
 * Copyright (C) 2008 Sergey Gridassov
 */

#include <stdio.h>
#include <stdlib.h>
#include "context.h"
#include "video.h"

#include "windows.h"
#include "cursor.h"
context_t *screen = NULL;
jump_table_t *jmptbl;

int main(int argc, char *argv[]) {
	if(argc < 5) {
		printf("Usage: %s <video driver> <width> <height> <bpp>\n", argv[0]);
		return 1;
	}

	printf("Grindars Window System starting\n");

	if(video_init(argv[1], atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), &screen, &jmptbl) == -1) {
		return 1;
	}


	cursor_init();

	windows_init();

	ProcessRedraw();
	return 0;
}

void assert_failed(const char *expression, const char *file, int line) {
	printf("gwinsy: assert failed! %s at %s:%d\n", expression, file, line);
	exit(1);
}

