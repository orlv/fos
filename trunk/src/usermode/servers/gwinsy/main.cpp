/*
 * Copyright (C) 2008 Sergey Gridassov
 */

#include <stdio.h>
#include <stdlib.h>
#include <fos/fos.h>
#include "context.h"
#include "video.h"
#include "input.h"
#include "windows.h"
#include "cursor.h"
#include "ipc.h"

context_t *screen = NULL;
jump_table_t *jmptbl;

int main(int argc, char *argv[]) {
	u32_t start  = uptime();

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
	input_init();
	ipc_init();

	u32_t time = uptime() - start;

	printf("gwinsy: started in %d ms\n", time);

	exec("/bin/test2", NULL);

	ProcessRedraw();
	return 0;
}

void assert_failed(const char *expression, const char *file, int line) {
	printf("gwinsy: assert failed! %s at %s:%d\n", expression, file, line);
	exit(1);
}

