#include <gui/types.h>
#include <stddef.h>
#define RED(x, bits)	((x >> (16 + 8 - bits)) & ((1 << bits) - 1))
#define GREEN(x, bits)	((x >> (8 + 8 - bits)) & ((1 << bits) - 1))
#define BLUE(x, bits)	((x >> (8 - bits)) & ((1 << bits) - 1))

extern unsigned short *lfb;
extern mode_definition_t __current_mode;
void SetPixel(int x, int y, int rgb, context_t *context)
{
	if(context == NULL) {
		if(x < 0 || y < 0 || x >= __current_mode.width || y >= __current_mode.height)return;
		lfb[x + y * __current_mode.width] = RED(rgb, 5) << 11 | GREEN(rgb, 6) << 5 | BLUE(rgb, 5);
	}else {
		if(x < 0 || y < 0 || x >= context->w || y >= context->h) return;
		unsigned short *ptr = (unsigned short *) context->data;
		ptr[x + y * context->w] = RED(rgb, 5) << 11 | GREEN(rgb, 6) << 5 | BLUE(rgb, 5);
	}
}

int GetPixel(int x, int y, context_t *context) {
	int color;
	if(context == NULL) {
		color =lfb[x + y * __current_mode.width];
		return (((color >> 11) & 0x1f) << (16 + 3)) | (((color >> 5) & 0x3f) << (8 + 2)) | ((color & 0x1f) << 3);
	}else  {
		unsigned short *ptr = (unsigned short *) context->data;
		color =ptr[x + y * context->w];
		if(context->native_pixels)
			return color;
		else
			return (((color >> 11) & 0x1f) << (16 + 3)) | (((color >> 5) & 0x3f) << (8 + 2)) | ((color & 0x1f) << 3);
	}

}
