/*
 *         FOS Graphics System
 * Copyright (c) 2007 Sergey Gridassov
 */

#include <fos/message.h>
#include <fos/fs.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <private/vbe.h>
#include <private/context.h>
#include <private/universalbpp.h>
#include <private/pixel.h>

#define VBESRV_CMD_SET_MODE (BASE_CMD_N + 0)

struct vbe_mode_info_block *vbe;
mode_callbacks *calltable;

struct  tmode_t {
	int	width;
	int	height;
	int	bpp;
	int	mode;
	mode_callbacks *clbk;
};

static mode_callbacks c16 = {
	FlushContext16,	border16,
	DrawRect16,	SetPixel16,
	GetPixel16,	
};

const static struct tmode_t modes[] = {
	{640,	480,	16,	0x4111,	&c16 },
	{800,	600,	16,	0x4114,	&c16 },
	{1024,	768,	16,	0x4117,	&c16 },
	{1280,	1024,	16,	0x411a,	&c16 },
	{0,	0,	0,	0,	NULL }
};

int init_video(int width, int height, int bpp, context_t *screen) {
	for(const struct tmode_t *ptr = modes; ptr->mode; ptr++) {
		if(ptr->width == width && ptr->height == height && ptr->bpp == bpp) {
			printf("GWinSy: Setting vesa mode 0x%x\n", ptr->mode);
			vbe_set_mode(ptr->mode);
			screen->w = width;
			screen->h = height;
			screen->bpp = bpp / 8;
			screen->native_pixels = 0;
			screen->data = kmmap(0, width * height * screen->bpp, 0, vbe->phys_base_addr);
			if(!screen->data)
				return 0;
			calltable = ptr->clbk;
			return 1;
		}
	}
	return 0;
}
void vbe_set_mode(u16_t mode)
{
  vbe = malloc(sizeof(struct vbe_mode_info_block));
  int fd;
  struct message msg;

  do {
    fd = open("/dev/vbe", 0);
  } while (fd == -1);
  msg.arg[0] = VBESRV_CMD_SET_MODE;
  msg.arg[1] = mode;
  msg.send_size = 0;
  msg.recv_size = sizeof(struct vbe_mode_info_block);
  msg.recv_buf = vbe;
  msg.tid = ((fd_t) fd)->thread;
  msg.flags = 0;
  send((struct message *)&msg);
  close(fd);
  if (!msg.arg[0]) {
    printf("Setting mode failed. :(\n");
    exit(1);
  }
}
