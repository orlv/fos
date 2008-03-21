/*
 * Copyright (C) 2008 Sergey Gridassov
 */

#include "config.h"
#if CONFIG_VESA == 1
#include <types.h>
#include <string.h>
#include <fos/message.h>
#include <fos/fs.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "context.h"
#include "vbe.h"

#define RED16(x, bits)	((x >> (16 + 8 - bits)) & ((1 << bits) - 1))
#define GREEN16(x, bits)	((x >> (8 + 8 - bits)) & ((1 << bits) - 1))
#define BLUE16(x, bits)	((x >> (8 - bits)) & ((1 << bits) - 1))

void Blit16(context_t *from, context_t *to, int x, int y, int w, int h, int src_x, int src_y)
{
  int y_limit = h + y;
  w *= 2;
  for (; y < y_limit; y++, src_y++) {
    u16_t *trg = to->data + y * to->w + x;
    u16_t *src = from->data + src_y * from->w + src_x;
    memcpy(trg, src, w);
  }
}

void SetPixel16(int x, int y, u32_t RGB, context_t * context)
{
  if (x < 0 || y < 0 || x >= context->w || y >= context->h)
    return;
  unsigned short *ptr = (unsigned short *)context->data;

  if (context->native_pixels)
    ptr[x + y * context->w] = RGB;
  else
    ptr[x + y * context->w] = RED16(RGB, 5) << 11 | GREEN16(RGB, 6) << 5 | BLUE16(RGB, 5);
}

u32_t GetPixel16(int x, int y, context_t * context)
{
  unsigned short *ptr = (unsigned short *)context->data;

  u32_t color = ptr[x + y * context->w];
  if (context->native_pixels)
    return color;
  else
    return (((color >> 11) & 0x1f) << (16 + 3)) | (((color >> 5) & 0x3f) << (8 + 2)) | ((color & 0x1f) << 3);
}

void Rect16(int x, int y, int w, int h, u32_t RGB, context_t * context)
{
  u16_t modecolor = 0;

  if (context->native_pixels)
    modecolor = RGB;
  else
    modecolor = RED16(RGB, 5) << 11 | GREEN16(RGB, 6) << 5 | BLUE16(RGB, 5);

  int y_limit = h + y;

  for (; y < y_limit; y++) {
    unsigned short *dot = (unsigned short *)context->data + y * context->w + x;	// * context->w

    for (int xx = 0; xx < w; xx++)
      *(dot++) = modecolor;
  }
}

struct {
	int width;
	int height;
	int bpp;
	u32_t mode;
	jump_table_t j;

} modes[] = {
/* { 640, 480, 15, 0x4110, { Blit15, SetPixel15, GetPixel15, Rect15, } }, */
   { 640, 480, 16, 0x4111, { Blit16, SetPixel16, GetPixel16, Rect16, } },
/* { 640, 480, 24, 0x4112, { Blit24, SetPixel24, GetPixel24, Rect24, } }, */

/* { 800, 600, 15, 0x4113, { Blit15, SetPixel15, GetPixel15, Rect15, } }, */
   { 800, 600, 16, 0x4114, { Blit16, SetPixel16, GetPixel16, Rect16, } },
/* { 800, 600, 24, 0x4115, { Blit24, SetPixel24, GetPixel24, Rect24, } }, */

/* { 1024, 768, 15, 0x4116, { Blit15, SetPixel15, GetPixel15, Rect15, } }, */
   { 1024, 768, 16, 0x4117, { Blit16, SetPixel16, GetPixel16, Rect16, } },
/* { 1024, 768, 24, 0x4118, { Blit24, SetPixel24, GetPixel24, Rect24, } }, */

/* { 1280, 1024, 15, 0x4119, { Blit15, SetPixel15, GetPixel15, Rect15, } }, */
   { 1280, 1024, 16, 0x411A, { Blit16, SetPixel16, GetPixel16, Rect16, } },
};

static int vbe_set_mode(u16_t mode, struct vbe_mode_info_block *vbe);

int vesa_init(int width, int height, int bpp, context_t **screen, jump_table_t **jmptbl) {
	for(int i = 0; i < sizeof(modes) / sizeof(modes[0]); i++) {
		if(modes[i].width == width && modes[i].height == height && modes[i].bpp == bpp) {
			struct vbe_mode_info_block vbe;
			if(vbe_set_mode(modes[i].mode, &vbe) == -1)
				return -1;
		
			*screen = malloc(sizeof(context_t));
			(*screen)->w = width;
			(*screen)->h = height;
			(*screen)->bpp = bpp;
			(*screen)->data = kmmap(0, width * height * bpp, 0, vbe.phys_base_addr);
			*jmptbl = &modes[i].j;
			return 0;
		}
	}
	return 0;
}

/* брр, гадость какая */
#define VBESRV_CMD_SET_MODE (BASE_CMD_N + 0)

static int vbe_set_mode(u16_t mode, struct vbe_mode_info_block *vbe)
{
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
  if (!msg.arg[0])
    return -1;
  return 0;
}


#endif

