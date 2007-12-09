#include <gui/types.h>
void border(int x, int y, int w, int h, context_t *context) {
	unsigned short *ptr;
	w--;
	h--;
	int pitch = context->w;
	ptr = (unsigned short *) context->data + x + y * pitch;
	for(int i = 0; i < w; i ++) {
		ptr[i] ^= 0xFFFF;
		ptr[h * pitch + i] ^= 0xFFFF;
	}
	for(int i = 0; i < h; i ++) {
		ptr[i * pitch] ^= 0xFFFF;
		ptr[i * pitch + w] ^= 0xFFFF;
	}
}
