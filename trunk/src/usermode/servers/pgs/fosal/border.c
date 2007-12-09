// XOR border
// не очень быстро...
#include <gui/types.h>
void border(int x, int y, int w, int h, context_t *context) {
	unsigned short *ptr;
	int pitch = context->w;
	// верхняя и нижняя линии
	ptr = (unsigned short *) context->data + x + y * pitch;	
	for(int i = 0; i < w; i ++) {
		ptr[i] ^= 0xFFFF;
		ptr[h * pitch + i] ^= 0xFFFF;
	}
	// левая и правая линии
	for(int i = 0; i < h; i ++) {
		ptr[i * pitch] ^= 0xFFFF;
		ptr[i * pitch + w] ^= 0xFFFF;
	}
}
