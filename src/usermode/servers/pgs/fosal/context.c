#include <stdio.h>
#include <string.h>
#include <gui/types.h>

void FlushContext(context_t  *context, int w, int h, int x, int y, int srcx, int srcy, context_t *target) {

	int y_limit = h + y;

	for(; y < y_limit; y++, srcy++) {
		unsigned short *trg = (unsigned short *)target->data + y * target->w + x; // 
		unsigned short *src = (unsigned short *)context->data + srcy  * context->w + srcx; //
		for(int xx = 0; xx < w; xx++)
			*(trg ++) = *(src ++);
	}
}
