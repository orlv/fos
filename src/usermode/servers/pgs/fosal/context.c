#include <stdio.h>
#include <string.h>
#include <gui/types.h>

extern unsigned short *lfb;
extern mode_definition_t __current_mode;

void FlushBackBuffer(char *back) {
	__asm("rep movsw"::"S"(back), "D"(lfb), "c"(__current_mode.width * __current_mode.height));
}
void FlushContext(context_t  *context, int w, int h, int x, int y, int srcx, int srcy, context_t *target) {
	int y_limit = h + y;
	if(target == NULL) {
		for(; y < y_limit; y++, srcy++) {
			unsigned short *trg = lfb + y * __current_mode.width + x;
			unsigned short *src = (unsigned short *)context->data + srcy * context->w + srcx;
			for(int xx = 0; xx < w; xx++) {
				*trg = *src;
				trg ++;
				src ++;
			}
		}
	}else {
		for(; y < y_limit; y++) {
			unsigned short *trg = (unsigned short *)target->data + y * target->w + x; // 
			unsigned short *src = (unsigned short *)context->data + srcy  * context->w + srcx; //
			for(int xx = 0; xx < w; xx++) {
				*trg = *src;
				trg ++;
				src ++;
			}
		}
	}
}
