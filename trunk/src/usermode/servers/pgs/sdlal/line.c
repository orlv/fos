/*
 * Portable Graphics System
 *  SDL abstraction layer
 * Copyright (c) 2007 Grindars
 */
#include <SDL.h>
 #include "setpixel.h"
SDL_Surface *screen;
void line(int x0, int y0, int x1, int y1, int color,  context_t *context) {
	int x, y, w, h;
	if(context == NULL) {
		if (SDL_MUSTLOCK(screen)) 
			SDL_LockSurface(screen);
		if(x0 < x1) {
			x = x0;
			w = x1 - x0;
		}else if (x0 == x1){
			x = x0;
			w = 1;
		}else {
			x = x1;
			w = x0 - x1;
		}

		if(y0 < y1) {
			y = y0;
			h = y1 - y0;
		}else if (y0 == y1){
			y = y0;
			h = 1;
		}else {
			y = y1;
			h = y0 - y1;
		}
	}
	int dy = y1 - y0;
	int dx = x1 - x0;
	int stepx, stepy;

	if (dy < 0) { dy = -dy;  stepy = -1; } else { stepy = 1; }
	if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }

	SetPixelInternal(x0, y0, color, context);
        SetPixelInternal(x1, y1, color, context);
	if (dx > dy) {
		int length = (dx - 1) >> 2;
		int extras = (dx - 1) & 3;
		int incr2 = (dy << 2) - (dx << 1);
		if (incr2 < 0) {
			int c = dy << 1;
			int incr1 = c << 1;
			int d =  incr1 - dx;
			for (int i = 0; i < length; i++) {
				x0 += stepx;
				x1 -= stepx;
				if (d < 0) {						// Pattern:
					SetPixelInternal(x0, y0, color, context);		//
					SetPixelInternal(x0 += stepx, y0, color, context);	//  x o o
					SetPixelInternal(x1, y1, color, context);		//
					SetPixelInternal(x1 -= stepx, y1, color, context);
					d += incr1;
				} else {
					if (d < c) {							// Pattern:
						SetPixelInternal(x0, y0, color, context);			//      o
						SetPixelInternal(x0 += stepx, y0 += stepy, color, context);	//  x o
						SetPixelInternal(x1, y1, color, context);			//
						SetPixelInternal(x1 -= stepx, y1 -= stepy, color, context);
					} else {
						SetPixelInternal(x0, y0 += stepy, color, context);	// Pattern:
						SetPixelInternal(x0 += stepx, y0, color, context);	//    o o 
						SetPixelInternal(x1, y1 -= stepy, color, context);	//  x
						SetPixelInternal(x1 -= stepx, y1, color, context);	//
					}
					d += incr2;
			}
		}
                if (extras > 0) {
                    if (d < 0) {
                        SetPixelInternal(x0 += stepx, y0, color, context);
                        if (extras > 1) SetPixelInternal(x0 += stepx, y0, color, context);
                        if (extras > 2) SetPixelInternal(x1 -= stepx, y1, color, context);
                    } else
                    if (d < c) {
                        SetPixelInternal(x0 += stepx, y0, color, context);
                        if (extras > 1) SetPixelInternal(x0 += stepx, y0 += stepy, color, context);
                        if (extras > 2) SetPixelInternal(x1 -= stepx, y1, color, context);
                    } else {
                        SetPixelInternal(x0 += stepx, y0 += stepy, color, context);
                        if (extras > 1) SetPixelInternal(x0 += stepx, y0, color, context);
                        if (extras > 2) SetPixelInternal(x1 -= stepx, y1 -= stepy, color, context);
                    }
                }
            } else {
                int c = (dy - dx) << 1;
                int incr1 = c << 1;
                int d =  incr1 + dx;
                for (int i = 0; i < length; i++) {
                    x0 += stepx;
                    x1 -= stepx;
                    if (d > 0) {
                        SetPixelInternal(x0, y0 += stepy, color, context);			// Pattern:
                        SetPixelInternal(x0 += stepx, y0 += stepy, color, context);		//      o
                        SetPixelInternal(x1, y1 -= stepy, color, context);			//    o
                        SetPixelInternal(x1 -= stepx, y1 -= stepy, color, context);		//  x
                        d += incr1;
                    } else {
                        if (d < c) {
                            SetPixelInternal(x0, y0, color, context);				// Pattern:
                            SetPixelInternal(x0 += stepx, y0 += stepy, color, context);       //      o
                            SetPixelInternal(x1, y1, color, context);                         //  x o
                            SetPixelInternal(x1 -= stepx, y1 -= stepy, color, context);       //
                        } else {
                            SetPixelInternal(x0, y0 += stepy, color, context);			// Pattern:
                            SetPixelInternal(x0 += stepx, y0, color, context);			//    o o
                            SetPixelInternal(x1, y1 -= stepy, color, context);			//  x
                            SetPixelInternal(x1 -= stepx, y1, color, context);			//
                        }
                        d += incr2;
                    }
                }
                if (extras > 0) {
                    if (d > 0) {
                        SetPixelInternal(x0 += stepx, y0 += stepy, color, context);
                        if (extras > 1) SetPixelInternal(x0 += stepx, y0 += stepy, color, context);
                        if (extras > 2) SetPixelInternal(x1 -= stepx, y1 -= stepy, color, context);
                    } else
                    if (d < c) {
                        SetPixelInternal(x0 += stepx, y0, color, context);
                        if (extras > 1) SetPixelInternal(x0 += stepx, y0 += stepy, color, context);
                        if (extras > 2) SetPixelInternal(x1 -= stepx, y1, color, context);
                    } else {
                        SetPixelInternal(x0 += stepx, y0 += stepy, color, context);
                        if (extras > 1) SetPixelInternal(x0 += stepx, y0, color, context);
                        if (extras > 2) {
                            if (d > c)
                                SetPixelInternal(x1 -= stepx, y1 -= stepy, color, context);
                            else
                                SetPixelInternal(x1 -= stepx, y1, color, context);
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
                int d =  incr1 - dy;
                for (int i = 0; i < length; i++) {
                    y0 += stepy;
                    y1 -= stepy;
                    if (d < 0) {
                        SetPixelInternal(x0, y0, color, context);
                        SetPixelInternal(x0, y0 += stepy, color, context);
                        SetPixelInternal(x1, y1, color, context);
                        SetPixelInternal(x1, y1 -= stepy, color, context);
                        d += incr1;
                    } else {
                        if (d < c) {
                            SetPixelInternal(x0, y0, color, context);
                            SetPixelInternal(x0 += stepx, y0 += stepy, color, context);
                            SetPixelInternal(x1, y1, color, context);
                            SetPixelInternal(x1 -= stepx, y1 -= stepy, color, context);
                        } else {
                            SetPixelInternal(x0 += stepx, y0, color, context);
                            SetPixelInternal(x0, y0 += stepy, color, context);
                            SetPixelInternal(x1 -= stepx, y1, color, context);
                            SetPixelInternal(x1, y1 -= stepy, color, context);
                        }
                        d += incr2;
                    }
                }
                if (extras > 0) {
                    if (d < 0) {
                        SetPixelInternal(x0, y0 += stepy, color, context);
                        if (extras > 1) SetPixelInternal(x0, y0 += stepy, color, context);
                        if (extras > 2) SetPixelInternal(x1, y1 -= stepy, color, context);
                    } else
                    if (d < c) {
                        SetPixelInternal(stepx, y0 += stepy, color, context);
                        if (extras > 1) SetPixelInternal(x0 += stepx, y0 += stepy, color, context);
                        if (extras > 2) SetPixelInternal(x1, y1 -= stepy, color, context);
                    } else {
                        SetPixelInternal(x0 += stepx, y0 += stepy, color, context);
                        if (extras > 1) SetPixelInternal(x0, y0 += stepy, color, context);
                        if (extras > 2) SetPixelInternal(x1 -= stepx, y1 -= stepy, color, context);
                    }
                }
            } else {
                int c = (dx - dy) << 1;
                int incr1 = c << 1;
                int d =  incr1 + dy;
                for (int i = 0; i < length; i++) {
                    y0 += stepy;
                    y1 -= stepy;
                    if (d > 0) {
                        SetPixelInternal(x0 += stepx, y0, color, context);
                        SetPixelInternal(x0 += stepx, y0 += stepy, color, context);
                        SetPixelInternal(x1 -= stepy, y1, color, context);
                        SetPixelInternal(x1 -= stepx, y1 -= stepy, color, context);
                        d += incr1;
                    } else {
                        if (d < c) {
                            SetPixelInternal(x0, y0, color, context);
                            SetPixelInternal(x0 += stepx, y0 += stepy, color, context);
                            SetPixelInternal(x1, y1, color, context);
                            SetPixelInternal(x1 -= stepx, y1 -= stepy, color, context);
                        } else {
                            SetPixelInternal(x0 += stepx, y0, color, context);
                            SetPixelInternal(x0, y0 += stepy, color, context);
                            SetPixelInternal(x1 -= stepx, y1, color, context);
                            SetPixelInternal(x1, y1 -= stepy, color, context);
                        }
                        d += incr2;
                    }
                }
                if (extras > 0) {
                    if (d > 0) {
                        SetPixelInternal(x0 += stepx, y0 += stepy, color, context);
                        if (extras > 1) SetPixelInternal(x0 += stepx, y0 += stepy, color, context);
                        if (extras > 2) SetPixelInternal(x1 -= stepx, y1 -= stepy, color, context);
                    } else
                    if (d < c) {
                        SetPixelInternal(x0, y0 += stepy, color, context);
                        if (extras > 1) SetPixelInternal(x0 += stepx, y0 += stepy, color, context);
                        if (extras > 2) SetPixelInternal(x1, y1 -= stepy, color, context);
                    } else {
                        SetPixelInternal(x0 += stepx, y0 += stepy, color, context);
                        if (extras > 1) SetPixelInternal(x0, y0 += stepy, color, context);
                        if (extras > 2) {
                            if (d > c)
                                SetPixelInternal(x1 -= stepx, y1 -= stepy, color, context);
                            else
                                SetPixelInternal(x1, y1 -= stepy, color, context);
                        }
                    }
                }
            }
        }
	if(context == NULL) {
		if (SDL_MUSTLOCK(screen)) 
			SDL_UnlockSurface(screen);

		SDL_UpdateRect(screen, x, y, w + 1, h + 1);
	}
}
