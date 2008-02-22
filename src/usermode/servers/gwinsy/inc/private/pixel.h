/*
 *         FOS Graphics System
 * Copyright (c) 2007 Sergey Gridassov
 */
#ifndef PIXEL_H
#define PIXEL_H
#include "types.h"
#include "universalbpp.h"
void SetPixel16(int x, int y, int rgb, context_t * context);
int GetPixel16 (int x, int y, context_t * context);
void DrawRect16(int x, int y, int width, int height, int color, context_t * context);
void PutString(int x, int y, char *str, int color, context_t * context);
void line(int x0, int y0, int x1, int y1, int color, context_t * context);
void border16(int x, int y, int w, int h, context_t * context);
#endif

