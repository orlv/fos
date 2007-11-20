#include <stdio.h>
#include <string.h>
#include <gui/types.h>

void FlushContext(context_t  *context, int w, int h, int x, int y, int srcx, int srcy, context_t *target) {
/*	if(x < 0) {
		srcx -= x;
		w += x;
		x = 0;
	}
	if(y < 0) {
		srcy -= y;
		h += y;
		y = 0;
	}
	if((y + h) >= target->h)
		h -= ((y + h) -  target->h);
	if((x + w) >= target->w) 
		w -= ((x + w) - target->w);*/
	int y_limit = h + y;

	for(; y < y_limit; y++, srcy++) {
		unsigned short *trg = (unsigned short *)target->data + y * target->w + x; // 
		unsigned short *src = (unsigned short *)context->data + srcy  * context->w + srcx; //
		for(int xx = 0; xx < w; xx++)
			*(trg ++) = *(src ++);
	}
}
