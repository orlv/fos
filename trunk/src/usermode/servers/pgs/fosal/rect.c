/*
 * Portable Graphics System
 *  FOS abstraction layer
 * Copyright (c) 2007 Grindars
 */
#include <gui/types.h>
#include <stddef.h>
#define RED(x, bits)	((x >> (16 + 8 - bits)) & ((1 << bits) - 1))
#define GREEN(x, bits)	((x >> (8 + 8 - bits)) & ((1 << bits) - 1))
#define BLUE(x, bits)	((x >> (8 - bits)) & ((1 << bits) - 1))
extern unsigned short *lfb;
extern mode_definition_t __current_mode;
 void DrawRect(int x, int y, int width, int height, int color, context_t *context)
{
	unsigned short modecolor = 0;
	if(context != NULL) {
	if(context->native_pixels) 
		modecolor = color;
	else
		modecolor = RED(color, 5) << 11 | GREEN(color, 6) << 5 | BLUE(color, 5);
	}else{
		modecolor = RED(color, 5) << 11 | GREEN(color, 6) << 5 | BLUE(color, 5);
	}

	int y_limit = height + y;
	if(context == NULL) {
		for(; y < y_limit; y++) {
			unsigned short *dot = lfb + y*__current_mode.width + x;
			for(int xx = 0; xx < width; xx++) 
				*(dot++) = modecolor;
		}
	} else {
		for(; y < y_limit; y++) {
			unsigned short *dot = (unsigned short *)context->data + y * context->w + x; // * context->w
			for(int xx = 0; xx < width; xx++) {
				*dot = modecolor;
				dot ++;
			}
		}
	}
		
}
