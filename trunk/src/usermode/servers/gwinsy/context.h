/*
 * Copyright (C) 2008 Sergey Gridassov
 */

#ifndef CONTEXT_H
#define CONTEXT_H

#include <types.h>

typedef struct {
  int w;
  int h;
  int bpp;
  int native_pixels;
  void *data;
} context_t;

typedef struct {
  void (*Blit)(context_t *from, context_t *to, int x, int y, int w, int h, int src_x, int src_y);
  void (*SetPixel)(int x, int y, u32_t RGB, context_t *context);
  u32_t (*GetPixel)(int x, int y, context_t *context);
  void (*Rect)(int x, int y, int w, int h, u32_t RGB, context_t *context);
} jump_table_t;
#endif
