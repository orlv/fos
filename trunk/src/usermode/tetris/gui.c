#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fos/message.h>
#include <fos/fs.h>
#include <sched.h>
#include <string.h>
#include "gui.h"
#define ALIGN(a, b)  ((a + (b - 1)) & ~(b - 1))

static volatile char *fnt = NULL;
static fd_t gui;
static fd_t gui_canvas;
void GUIInit() {
	int gui_handle;
	do {
		gui_handle = open("/dev/pgs", 0);
		sched_yield();
	} while(!gui_handle);
	gui = (fd_t )gui_handle;

	int gui_canvas_h;
	do {
		gui_canvas_h = open("/dev/pgs_canvas", 0);
		sched_yield();
	} while(!gui_handle);
	gui_canvas = (fd_t )gui_canvas_h; 
}
typedef struct {
	int w;
	int h;
	int class;
} create_win_t;

struct win_hndl{
	int handle;
	char *data;
	int w;
	int h;
	int bpp;
	int margin_up;
	int margin_down;
	int margin_left;
	int margin_right;
} ;

struct win_info {
	int bpp;
	int handle;
	int margin_up;
	int margin_down;
	int margin_left;
	int margin_right;
};
#define MAX_TITLE_LEN 64

int CreateWindow(int w, int h, char *caption, int flags) {
	char *buf = malloc(sizeof(create_win_t) + MAX_TITLE_LEN);
	create_win_t *struc = (create_win_t *) buf;
	char *title = buf + sizeof(create_win_t);
	strcpy(title, caption);
	struc->w = w;
	struc->h = h;
	struc->class = flags;
	

	struct win_info wi;

	struct message msg;
	msg.recv_size = sizeof(wi);
	msg.recv_buf = &wi;
	msg.tid = gui->thread;
	msg.flags = 0;
	msg.arg[0] = WIN_CMD_CREATEWINDOW;
	msg.send_size = sizeof(create_win_t) + MAX_TITLE_LEN;
	msg.send_buf = buf;
	send(&msg);
	int hndl = wi.handle;

	int size = ALIGN((w + wi.margin_left + wi.margin_right) * (h + wi.margin_up + wi.margin_down) * wi.bpp, 4096);
	char *canvas = kmmap(0, size, 0, 0);
	msg.tid = gui_canvas->thread;
	msg.recv_size =  size;
	msg.recv_buf = canvas;
	msg.send_size = size;
	msg.send_buf = canvas;
	msg.flags = MSG_MEM_SHARE;
	msg.arg[0] = WIN_CMD_MAPBUF;
	msg.arg[1] = hndl;

	send(&msg);

	struct win_hndl *wh = malloc(sizeof(struct win_hndl));
	wh->margin_up = wi.margin_up;
	wh->margin_down = wi.margin_down;
	wh->margin_left = wi.margin_left;
	wh->margin_right = wi.margin_right;
	wh->handle = hndl;
	wh->data = canvas;
	wh->w = w;
	wh->h = h;
	wh->bpp = wi.bpp;
	return (int) wh;
}
typedef struct {
	void * reserved[2];
	int class;
	int handle;
	int a0;
	int a1;
	int a2;
	int a3;
} event_t;
void WaitEvent(int *class, int *handle, int *a0, int *a1, int *a2, int *a3) {
	event_t event;
	struct message msg;
	msg.flags = 0;
	msg.arg[0] = WIN_CMD_WAIT_EVENT;
	msg.tid = gui->thread;
	msg.send_size = 0;
	msg.recv_size = sizeof(event_t);
	msg.recv_buf = &event;
	send(&msg);
	*class = event.class;
	*handle = event.handle;
	*a0 = event.a0;
	*a1 = event.a1;
	*a2 = event.a2;
	*a3 = event.a3;


}
void DestroyWindow(int handle) {
	struct win_hndl *wh  = (struct win_hndl *) handle;
	struct message msg;
	msg.flags = 0;
	msg.send_size = 0;
	msg.recv_size = 0;
	msg.tid = gui->thread;
	msg.arg[0] = WIN_CMD_DESTROYWINDOW;
	msg.arg[1] = wh->handle;
	send(&msg);
	free(wh);
	kmunmap((off_t) wh->data, ALIGN(wh->w * wh->h * wh->bpp, 4096));
}
void RefreshWindow(int handle) {
	struct win_hndl *wh  = (struct win_hndl *) handle;
	struct message msg;
	msg.flags = 0;
	msg.send_size = 0;
	msg.recv_size = 0;
	msg.tid = gui->thread;
	msg.arg[0] = WIN_CMD_REFRESHWINDOW;
	msg.arg[1] = wh->handle;
	send(&msg);
}
void GuiEnd() {
	struct message msg;
	msg.flags = 0;
	msg.send_size = 0;
	msg.recv_size = 0;
	msg.tid = gui->thread;
	msg.arg[0] = WIN_CMD_CLEANUP;
	send(&msg);
	gui = NULL;
	gui_canvas = NULL;
	if(fnt)
		free(fnt);
}
#define RED(x, bits)	((x >> (16 + 8 - bits)) & ((1 << bits) - 1))
#define GREEN(x, bits)	((x >> (8 + 8 - bits)) & ((1 << bits) - 1))
#define BLUE(x, bits)	((x >> (8 - bits)) & ((1 << bits) - 1))
void pixel(int handle, int x, int y, int color) {
	struct win_hndl *wh  = (struct win_hndl *) handle;
	x += wh->margin_left;
	y += wh->margin_up;
	if(x > wh->w - wh->margin_right || y > wh->h - wh->margin_down || y < 0 || x < 0) return;
	unsigned short *ptr = (unsigned short *) wh->data;
	ptr[x + y * (wh->w + wh->margin_left + wh->margin_right)] =  RED(color, 5) << 11 | GREEN(color, 6) << 5 | BLUE(color, 5);
}

void rect(int handle, int x, int y, int width, int height, int color)
{
	struct win_hndl *wh  = (struct win_hndl *) handle;
	x += wh->margin_left;
	y += wh->margin_up;

	unsigned short modecolor = RED(color, 5) << 11 | GREEN(color, 6) << 5 | BLUE(color, 5);

	int y_limit = height + y;

	for(; y < y_limit; y++) {
		unsigned short *dot = (unsigned short *)wh->data + y * (wh->w + wh->margin_left + wh->margin_right) + x; // * context->w
		for(int xx = 0; xx < width; xx++) 
			*(dot++) = modecolor;
	}
	
}
void pstring(int handle, int x, int y, int color, char *str) {
	if(!fnt) {
		printf("Loading on request\n");
		fnt = malloc(4096);
		int h = open("/mnt/modules/font.psf", 0);
		lseek(h, 4, SEEK_SET);
		read(h, fnt, 4096);
		close(h);
		
	}
	for(;*str; str++) {
		for(int i = 0; i < 16; i++)
		for(int j = 0; j < 8; j++) 
			if(fnt[16 * (unsigned char)*str + i] & (1<<j))
				//__asm__("nop");
				pixel(handle, x + 8 - j, y + i, color);
		x += 8;
	}
}
#define SetPixel(a, b, c, d) pixel(d, a, b, c)
void line(int handle, int x0, int y0, int x1, int y1, int color) {
	int dy = y1 - y0;
	int dx = x1 - x0;
	int stepx, stepy;

	if (dy < 0) { dy = -dy;  stepy = -1; } else { stepy = 1; }
	if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }

	SetPixel(x0, y0, color, handle);
        SetPixel(x1, y1, color, handle);
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
					SetPixel(x0, y0, color, handle);		//
					SetPixel(x0 += stepx, y0, color, handle);	//  x o o
					SetPixel(x1, y1, color, handle);		//
					SetPixel(x1 -= stepx, y1, color, handle);
					d += incr1;
				} else {
					if (d < c) {							// Pattern:
						SetPixel(x0, y0, color, handle);			//      o
						SetPixel(x0 += stepx, y0 += stepy, color, handle);	//  x o
						SetPixel(x1, y1, color, handle);			//
						SetPixel(x1 -= stepx, y1 -= stepy, color, handle);
					} else {
						SetPixel(x0, y0 += stepy, color, handle);	// Pattern:
						SetPixel(x0 += stepx, y0, color, handle);	//    o o 
						SetPixel(x1, y1 -= stepy, color, handle);	//  x
						SetPixel(x1 -= stepx, y1, color, handle);	//
					}
					d += incr2;
			}
		}
                if (extras > 0) {
                    if (d < 0) {
                        SetPixel(x0 += stepx, y0, color, handle);
                        if (extras > 1) SetPixel(x0 += stepx, y0, color, handle);
                        if (extras > 2) SetPixel(x1 -= stepx, y1, color, handle);
                    } else
                    if (d < c) {
                        SetPixel(x0 += stepx, y0, color, handle);
                        if (extras > 1) SetPixel(x0 += stepx, y0 += stepy, color, handle);
                        if (extras > 2) SetPixel(x1 -= stepx, y1, color, handle);
                    } else {
                        SetPixel(x0 += stepx, y0 += stepy, color, handle);
                        if (extras > 1) SetPixel(x0 += stepx, y0, color, handle);
                        if (extras > 2) SetPixel(x1 -= stepx, y1 -= stepy, color, handle);
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
                        SetPixel(x0, y0 += stepy, color, handle);			// Pattern:
                        SetPixel(x0 += stepx, y0 += stepy, color, handle);		//      o
                        SetPixel(x1, y1 -= stepy, color, handle);			//    o
                        SetPixel(x1 -= stepx, y1 -= stepy, color, handle);		//  x
                        d += incr1;
                    } else {
                        if (d < c) {
                            SetPixel(x0, y0, color, handle);				// Pattern:
                            SetPixel(x0 += stepx, y0 += stepy, color, handle);       //      o
                            SetPixel(x1, y1, color, handle);                         //  x o
                            SetPixel(x1 -= stepx, y1 -= stepy, color, handle);       //
                        } else {
                            SetPixel(x0, y0 += stepy, color, handle);			// Pattern:
                            SetPixel(x0 += stepx, y0, color, handle);			//    o o
                            SetPixel(x1, y1 -= stepy, color, handle);			//  x
                            SetPixel(x1 -= stepx, y1, color, handle);			//
                        }
                        d += incr2;
                    }
                }
                if (extras > 0) {
                    if (d > 0) {
                        SetPixel(x0 += stepx, y0 += stepy, color, handle);
                        if (extras > 1) SetPixel(x0 += stepx, y0 += stepy, color, handle);
                        if (extras > 2) SetPixel(x1 -= stepx, y1 -= stepy, color, handle);
                    } else
                    if (d < c) {
                        SetPixel(x0 += stepx, y0, color, handle);
                        if (extras > 1) SetPixel(x0 += stepx, y0 += stepy, color, handle);
                        if (extras > 2) SetPixel(x1 -= stepx, y1, color, handle);
                    } else {
                        SetPixel(x0 += stepx, y0 += stepy, color, handle);
                        if (extras > 1) SetPixel(x0 += stepx, y0, color, handle);
                        if (extras > 2) {
                            if (d > c)
                                SetPixel(x1 -= stepx, y1 -= stepy, color, handle);
                            else
                                SetPixel(x1 -= stepx, y1, color, handle);
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
                        SetPixel(x0, y0, color, handle);
                        SetPixel(x0, y0 += stepy, color, handle);
                        SetPixel(x1, y1, color, handle);
                        SetPixel(x1, y1 -= stepy, color, handle);
                        d += incr1;
                    } else {
                        if (d < c) {
                            SetPixel(x0, y0, color, handle);
                            SetPixel(x0 += stepx, y0 += stepy, color, handle);
                            SetPixel(x1, y1, color, handle);
                            SetPixel(x1 -= stepx, y1 -= stepy, color, handle);
                        } else {
                            SetPixel(x0 += stepx, y0, color, handle);
                            SetPixel(x0, y0 += stepy, color, handle);
                            SetPixel(x1 -= stepx, y1, color, handle);
                            SetPixel(x1, y1 -= stepy, color, handle);
                        }
                        d += incr2;
                    }
                }
                if (extras > 0) {
                    if (d < 0) {
                        SetPixel(x0, y0 += stepy, color, handle);
                        if (extras > 1) SetPixel(x0, y0 += stepy, color, handle);
                        if (extras > 2) SetPixel(x1, y1 -= stepy, color, handle);
                    } else
                    if (d < c) {
                        SetPixel(stepx, y0 += stepy, color, handle);
                        if (extras > 1) SetPixel(x0 += stepx, y0 += stepy, color, handle);
                        if (extras > 2) SetPixel(x1, y1 -= stepy, color, handle);
                    } else {
                        SetPixel(x0 += stepx, y0 += stepy, color, handle);
                        if (extras > 1) SetPixel(x0, y0 += stepy, color, handle);
                        if (extras > 2) SetPixel(x1 -= stepx, y1 -= stepy, color, handle);
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
                        SetPixel(x0 += stepx, y0, color, handle);
                        SetPixel(x0 += stepx, y0 += stepy, color, handle);
                        SetPixel(x1 -= stepy, y1, color, handle);
                        SetPixel(x1 -= stepx, y1 -= stepy, color, handle);
                        d += incr1;
                    } else {
                        if (d < c) {
                            SetPixel(x0, y0, color, handle);
                            SetPixel(x0 += stepx, y0 += stepy, color, handle);
                            SetPixel(x1, y1, color, handle);
                            SetPixel(x1 -= stepx, y1 -= stepy, color, handle);
                        } else {
                            SetPixel(x0 += stepx, y0, color, handle);
                            SetPixel(x0, y0 += stepy, color, handle);
                            SetPixel(x1 -= stepx, y1, color, handle);
                            SetPixel(x1, y1 -= stepy, color, handle);
                        }
                        d += incr2;
                    }
                }
                if (extras > 0) {
                    if (d > 0) {
                        SetPixel(x0 += stepx, y0 += stepy, color, handle);
                        if (extras > 1) SetPixel(x0 += stepx, y0 += stepy, color, handle);
                        if (extras > 2) SetPixel(x1 -= stepx, y1 -= stepy, color, handle);
                    } else
                    if (d < c) {
                        SetPixel(x0, y0 += stepy, color, handle);
                        if (extras > 1) SetPixel(x0 += stepx, y0 += stepy, color, handle);
                        if (extras > 2) SetPixel(x1, y1 -= stepy, color, handle);
                    } else {
                        SetPixel(x0 += stepx, y0 += stepy, color, handle);
                        if (extras > 1) SetPixel(x0, y0 += stepy, color, handle);
                        if (extras > 2) {
                            if (d > c)
                                SetPixel(x1 -= stepx, y1 -= stepy, color, handle);
                            else
                                SetPixel(x1, y1 -= stepy, color, handle);
                        }
                    }
                }
            }
        }
}
