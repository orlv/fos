/*
 *         FOS Graphics System
 * Copyright (c) 2007 Sergey Gridassov
 */

#include <private/pixel.h>
#define RED(x, bits)	((x >> (16 + 8 - bits)) & ((1 << bits) - 1))
#define GREEN(x, bits)	((x >> (8 + 8 - bits)) & ((1 << bits) - 1))
#define BLUE(x, bits)	((x >> (8 - bits)) & ((1 << bits) - 1))
void line(int x0, int y0, int x1, int y1, int color, context_t * context)
{
  int dy = y1 - y0;
  int dx = x1 - x0;
  int stepx, stepy;

  if (dy < 0) {
    dy = -dy;
    stepy = -1;
  } else {
    stepy = 1;
  }
  if (dx < 0) {
    dx = -dx;
    stepx = -1;
  } else {
    stepx = 1;
  }
  if(dy == 0) {
    unsigned short modecolor = RED(color, 5) << 11 | GREEN(color, 6) << 5 | BLUE(color, 5);

    unsigned short *dot = (unsigned short *)context->data + y0 * context->w + x0;
    if(stepx > 0)
      for(int xx = x0; xx <= x1; xx++)
        *(dot++) = modecolor;
    else
      for(int xx = x1; xx <= x0; xx++)
        *(dot++) = modecolor;
    return;
  }
  if(dx == 0) {
    unsigned short modecolor = RED(color, 5) << 11 | GREEN(color, 6) << 5 | BLUE(color, 5);
    unsigned short *dot = (unsigned short *)context->data + y0 * context->w + x0;
    if(stepy > 0)
      for(int yy = y0; yy <= y1; yy++) {
        *dot = modecolor;
        dot+=context->w;
      }
    else
      for(int yy = y1; yy <= y0; yy++) {
        *dot = modecolor;
        dot-=context->w;
      }
    return;
  }
  SetPixel(x0, y0, color, context);
  SetPixel(x1, y1, color, context);
  if (dx > dy) {
    int length = (dx - 1) >> 2;
    int extras = (dx - 1) & 3;
    int incr2 = (dy << 2) - (dx << 1);

    if (incr2 < 0) {
      int c = dy << 1;
      int incr1 = c << 1;
      int d = incr1 - dx;

      for (int i = 0; i < length; i++) {
	x0 += stepx;
	x1 -= stepx;
	if (d < 0) {		// Pattern:
	  SetPixel(x0, y0, color, context);	//
	  SetPixel(x0 += stepx, y0, color, context);	//  x o o
	  SetPixel(x1, y1, color, context);	//
	  SetPixel(x1 -= stepx, y1, color, context);
	  d += incr1;
	} else {
	  if (d < c) {		// Pattern:
	    SetPixel(x0, y0, color, context);	//      o
	    SetPixel(x0 += stepx, y0 += stepy, color, context);	//  x o
	    SetPixel(x1, y1, color, context);	//
	    SetPixel(x1 -= stepx, y1 -= stepy, color, context);
	  } else {
	    SetPixel(x0, y0 += stepy, color, context);	// Pattern:
	    SetPixel(x0 += stepx, y0, color, context);	//    o o 
	    SetPixel(x1, y1 -= stepy, color, context);	//  x
	    SetPixel(x1 -= stepx, y1, color, context);	//
	  }
	  d += incr2;
	}
      }
      if (extras > 0) {
	if (d < 0) {
	  SetPixel(x0 += stepx, y0, color, context);
	  if (extras > 1)
	    SetPixel(x0 += stepx, y0, color, context);
	  if (extras > 2)
	    SetPixel(x1 -= stepx, y1, color, context);
	} else if (d < c) {
	  SetPixel(x0 += stepx, y0, color, context);
	  if (extras > 1)
	    SetPixel(x0 += stepx, y0 += stepy, color, context);
	  if (extras > 2)
	    SetPixel(x1 -= stepx, y1, color, context);
	} else {
	  SetPixel(x0 += stepx, y0 += stepy, color, context);
	  if (extras > 1)
	    SetPixel(x0 += stepx, y0, color, context);
	  if (extras > 2)
	    SetPixel(x1 -= stepx, y1 -= stepy, color, context);
	}
      }
    } else {
      int c = (dy - dx) << 1;
      int incr1 = c << 1;
      int d = incr1 + dx;

      for (int i = 0; i < length; i++) {
	x0 += stepx;
	x1 -= stepx;
	if (d > 0) {
	  SetPixel(x0, y0 += stepy, color, context);	// Pattern:
	  SetPixel(x0 += stepx, y0 += stepy, color, context);	//      o
	  SetPixel(x1, y1 -= stepy, color, context);	//    o
	  SetPixel(x1 -= stepx, y1 -= stepy, color, context);	//  x
	  d += incr1;
	} else {
	  if (d < c) {
	    SetPixel(x0, y0, color, context);	// Pattern:
	    SetPixel(x0 += stepx, y0 += stepy, color, context);	//      o
	    SetPixel(x1, y1, color, context);	//  x o
	    SetPixel(x1 -= stepx, y1 -= stepy, color, context);	//
	  } else {
	    SetPixel(x0, y0 += stepy, color, context);	// Pattern:
	    SetPixel(x0 += stepx, y0, color, context);	//    o o
	    SetPixel(x1, y1 -= stepy, color, context);	//  x
	    SetPixel(x1 -= stepx, y1, color, context);	//
	  }
	  d += incr2;
	}
      }
      if (extras > 0) {
	if (d > 0) {
	  SetPixel(x0 += stepx, y0 += stepy, color, context);
	  if (extras > 1)
	    SetPixel(x0 += stepx, y0 += stepy, color, context);
	  if (extras > 2)
	    SetPixel(x1 -= stepx, y1 -= stepy, color, context);
	} else if (d < c) {
	  SetPixel(x0 += stepx, y0, color, context);
	  if (extras > 1)
	    SetPixel(x0 += stepx, y0 += stepy, color, context);
	  if (extras > 2)
	    SetPixel(x1 -= stepx, y1, color, context);
	} else {
	  SetPixel(x0 += stepx, y0 += stepy, color, context);
	  if (extras > 1)
	    SetPixel(x0 += stepx, y0, color, context);
	  if (extras > 2) {
	    if (d > c)
	      SetPixel(x1 -= stepx, y1 -= stepy, color, context);
	    else
	      SetPixel(x1 -= stepx, y1, color, context);
	  }
	}
      }
    }
  } else {
    int length = (dy - 1) >> 2;
    int extras = (dy - 1) & 3;
    int incr2 = (dx << 2) - (dy << 1);

    if (incr2 < 0) {
      int c = dx << 1;
      int incr1 = c << 1;
      int d = incr1 - dy;

      for (int i = 0; i < length; i++) {
	y0 += stepy;
	y1 -= stepy;
	if (d < 0) {
	  SetPixel(x0, y0, color, context);
	  SetPixel(x0, y0 += stepy, color, context);
	  SetPixel(x1, y1, color, context);
	  SetPixel(x1, y1 -= stepy, color, context);
	  d += incr1;
	} else {
	  if (d < c) {
	    SetPixel(x0, y0, color, context);
	    SetPixel(x0 += stepx, y0 += stepy, color, context);
	    SetPixel(x1, y1, color, context);
	    SetPixel(x1 -= stepx, y1 -= stepy, color, context);
	  } else {
	    SetPixel(x0 += stepx, y0, color, context);
	    SetPixel(x0, y0 += stepy, color, context);
	    SetPixel(x1 -= stepx, y1, color, context);
	    SetPixel(x1, y1 -= stepy, color, context);
	  }
	  d += incr2;
	}
      }
      if (extras > 0) {
	if (d < 0) {
	  SetPixel(x0, y0 += stepy, color, context);
	  if (extras > 1)
	    SetPixel(x0, y0 += stepy, color, context);
	  if (extras > 2)
	    SetPixel(x1, y1 -= stepy, color, context);
	} else if (d < c) {
	  SetPixel(stepx, y0 += stepy, color, context);
	  if (extras > 1)
	    SetPixel(x0 += stepx, y0 += stepy, color, context);
	  if (extras > 2)
	    SetPixel(x1, y1 -= stepy, color, context);
	} else {
	  SetPixel(x0 += stepx, y0 += stepy, color, context);
	  if (extras > 1)
	    SetPixel(x0, y0 += stepy, color, context);
	  if (extras > 2)
	    SetPixel(x1 -= stepx, y1 -= stepy, color, context);
	}
      }
    } else {
      int c = (dx - dy) << 1;
      int incr1 = c << 1;
      int d = incr1 + dy;

      for (int i = 0; i < length; i++) {
	y0 += stepy;
	y1 -= stepy;
	if (d > 0) {
	  SetPixel(x0 += stepx, y0, color, context);
	  SetPixel(x0 += stepx, y0 += stepy, color, context);
	  SetPixel(x1 -= stepy, y1, color, context);
	  SetPixel(x1 -= stepx, y1 -= stepy, color, context);
	  d += incr1;
	} else {
	  if (d < c) {
	    SetPixel(x0, y0, color, context);
	    SetPixel(x0 += stepx, y0 += stepy, color, context);
	    SetPixel(x1, y1, color, context);
	    SetPixel(x1 -= stepx, y1 -= stepy, color, context);
	  } else {
	    SetPixel(x0 += stepx, y0, color, context);
	    SetPixel(x0, y0 += stepy, color, context);
	    SetPixel(x1 -= stepx, y1, color, context);
	    SetPixel(x1, y1 -= stepy, color, context);
	  }
	  d += incr2;
	}
      }
      if (extras > 0) {
	if (d > 0) {
	  SetPixel(x0 += stepx, y0 += stepy, color, context);
	  if (extras > 1)
	    SetPixel(x0 += stepx, y0 += stepy, color, context);
	  if (extras > 2)
	    SetPixel(x1 -= stepx, y1 -= stepy, color, context);
	} else if (d < c) {
	  SetPixel(x0, y0 += stepy, color, context);
	  if (extras > 1)
	    SetPixel(x0 += stepx, y0 += stepy, color, context);
	  if (extras > 2)
	    SetPixel(x1, y1 -= stepy, color, context);
	} else {
	  SetPixel(x0 += stepx, y0 += stepy, color, context);
	  if (extras > 1)
	    SetPixel(x0, y0 += stepy, color, context);
	  if (extras > 2) {
	    if (d > c)
	      SetPixel(x1 -= stepx, y1 -= stepy, color, context);
	    else
	      SetPixel(x1, y1 -= stepy, color, context);
	  }
	}
      }
    }
  }
}
