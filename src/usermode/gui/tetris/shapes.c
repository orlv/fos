/*
  Copyright (C) 2007 Serge Gridassov
 */

#include <string.h>
#include <stdlib.h>
#include "shapes.h"

#define	TL	-B_COLS-1	/* top left */
#define	TC	-B_COLS		/* top center */
#define	TR	-B_COLS+1	/* top right */
#define	ML	-1		/* middle left */
#define	MR	1		/* middle right */
#define	BL	B_COLS-1	/* bottom left */
#define	BC	B_COLS		/* bottom center */
#define	BR	B_COLS+1	/* bottom right */

struct shape shapes[] = {
  /* 0 */ {7, {TL, TC, MR,}},
  /* 1 */ {8, {TC, TR, ML,}},
  /* 2 */ {9, {ML, MR, BC,}},
  /* 3 */ {3, {TL, TC, ML,}},
  /* 4 */ {12, {ML, BL, MR,}},
  /* 5 */ {15, {ML, BR, MR,}},
  /* 6 */ {18, {ML, MR, 2}},         /* sticks out */
  /* 7 */ {0, {TC, ML, BL,}},
  /* 8 */ {1, {TC, MR, BR,}},
  /* 9 */ {10, {TC, MR, BC,}},
  /*10 */ {11, {TC, ML, MR,}},
  /*11 */ {2, {TC, ML, BC,}},
  /*12 */ {13, {TC, BC, BR,}},
  /*13 */ {14, {TR, ML, MR,}},
  /*14 */ {4, {TL, TC, BC,}},
  /*15 */ {16, {TR, TC, BC,}},
  /*16 */ {17, {TL, MR, ML,}},
  /*17 */ {5, {TC, BC, BL,}},
  /*18 */ {6, {TC, BC, 2 * B_COLS}} /* sticks out */
};

const unsigned long colors[] = {
  0x800000, 0x008000, 0x000080,
};

void pcolor(struct shape *shape)
{
  shape->color = colors[random() % 3];
}

void place(struct shape *shape, int pos, int onoff)
{
  int *o = shape->off;

  board[pos] = onoff ? shape->color : 0;
  board[pos + *o++] = onoff ? shape->color : 0;
  board[pos + *o++] = onoff ? shape->color : 0;
  board[pos + *o] = onoff ? shape->color : 0;
}

int fits_in(struct shape *shape, int pos)
{
  int *o = shape->off;

  if (board[pos] || board[pos + *o++] || board[pos + *o++] || board[pos + *o])
    return 0;
  return 1;
}

void elide()
{
  int i, j, base;
  unsigned long *p;

  for (i = A_FIRST; i < A_LAST; i++) {
    base = i * B_COLS + 1;
    p = &board[base];
    for (j = B_COLS - 2; *p++ != 0;) {
      if (--j <= 0) {
	/* this row is to be elided */
	memset(&board[base], 0, (B_COLS - 2) * 4);
	redraw();
	tsleep();
	while (--base != 0)
	  board[base + B_COLS] = board[base];
	redraw();
	tsleep();
	break;
      }
    }
  }
}
