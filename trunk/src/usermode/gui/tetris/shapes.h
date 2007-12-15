#ifndef SHAPES_H
#define SHAPES_H
#include <fos/fos.h>
struct shape {
	int	rot;	/* index of rotated version of this shape */
	int	off[3];	/* offsets to other blots if center is at (0,0) */
	unsigned long color;
};
extern unsigned long board[];
extern  struct shape *curshape;
void place(struct shape *shape, int pos, int onoff);
extern struct shape shapes[];

#define	randshape() (&shapes[random() % 7])

#define B_COLS 12
#define B_ROWS 23
#define B_SIZE (B_ROWS * B_COLS)
#define	A_FIRST	1
#define	A_LAST	21

extern void redraw();
extern int tsleep();
int fits_in(struct shape *shape, int pos);
void elide();
void pcolor(struct shape *shape);
#endif
