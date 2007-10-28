/*
 * Portable Graphics System
 *       GUI system
 * Copyright (c) 2007 Grindars
 */
// прототипы функций из AL
#ifndef GUI_AL_H
#define GUI_AL_H
#include <gui/types.h>
mode_definition_t graphics_init();
void StartEventHandling();
void DrawRect(int x, int y, int width, int height, int color, context_t  *context);
void DrawImage(int x, int y, picture_t *pict,  context_t  *context);
void line(int x0, int y0, int x1, int y1, int color, context_t  *context);
void PutString(int x, int y, char * str, int color,  context_t  *context);
void FlushContext(context_t  *context, int w, int h, int x, int y, int srcx, int srcy, context_t *target);
void FlushBackBuffer(char *back);
int GetPixel(int x, int y, context_t *context);
void SetPixel(int x, int y, int rgb, volatile context_t *context);
void *RequestMemory(int c);
void PostEvent(int tid, int handle, int class, int a0, int a1, int a2, int a3);
#endif
