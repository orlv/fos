#include <fos/fos.h>
#include <fos/message.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "gui.h"
#include "shapes.h"
#define KUP 1
#define KDOWN 2
#define KLEFT 3
#define KRIGHT 4

volatile int key = 0;
int winhandle;
unsigned long board[B_SIZE];
struct shape *curshape;
 struct shape *nextshape;
int seed = -1;
int score = 0;
void redraw() {
	rect(winhandle, 8, 8, 192, 368, 0x7F7F7F);
	for(int j = 0; j < B_ROWS; j++)
	for(int i = 0; i < B_COLS; i++) 
		if(board[j * B_COLS + i])
			rect(winhandle, 8 + i * 16, 8 + j * 16, 16, 16, board[j * B_COLS + i]);

	RefreshWindow(winhandle);
	
}
static void setup_board()
{
	int i;
	unsigned long *p;

	p = board;
	for (i = B_SIZE; i; i--)
		*p++ = i <= (2 * B_COLS) || (i % B_COLS) < 2;
}
int tsleep() {
	struct message msg;
	msg.tid = _MSG_SENDER_ANY;
	msg.recv_buf = NULL;
	msg.recv_size = 0;
	msg.flags = 0;
	alarm(200);
	receive(&msg);
	alarm(0);
	reply(&msg);
	return !(msg.tid == _MSG_SENDER_SIGNAL);
}
void game_thread() {
	int pos;
	setup_board();
	redraw();
	curshape = randshape();
	nextshape = randshape();
	pcolor(curshape);
	pcolor(nextshape);
	pos = A_FIRST*B_COLS + (B_COLS/2)-1;
	for(;;) {
		place(curshape, pos, 1);
		redraw();
		int code = tsleep();
		place(curshape, pos, 0);
		if(!code) {
			if (fits_in(curshape, pos + B_COLS)) {
				pos += B_COLS;
				continue;
			}
			place(curshape, pos, 1);
			score++;
			elide();
			curshape = nextshape;
			nextshape = randshape();
			pcolor(nextshape);
			pos = A_FIRST*B_COLS + (B_COLS/2)-1;
			if (!fits_in(curshape, pos))
				break;
			continue;
		} else {
			if(key == KLEFT) {
				if (fits_in(curshape, pos - 1))
				pos--;
				key = 0;
				continue;
			}
			if(key == KUP) {
				struct shape *new = &shapes[curshape->rot];
				new->color = curshape->color;
				if (fits_in(new, pos))
					curshape = new;
				key = 0;
				continue;
			}
			if(key == KRIGHT) {
				if (fits_in(curshape, pos + 1))
				pos++;
				key = 0;
				continue;
			}
			if (key == KDOWN) {
				while (fits_in(curshape, pos + B_COLS)) {
					pos += B_COLS;
					score++;
				}
				key = 0;
				continue;
			}
			key = 0;
		}
	}
	printf("game over.\n");
	exit(1);
}
asmlinkage int main(int argc, char ** argv)
{
	GUIInit();
	winhandle = CreateWindow(192 + 16,368 + 16, "Tetris - use arrows", WC_WINDOW);
	RefreshWindow(winhandle);
	int game = thread_create((off_t) game_thread);

	int class, handle, a0, a1, a2, a3;
	while(1) {
		WaitEvent(&class, &handle, &a0, &a1, &a2, &a3);
		switch(class) {
		case  EV_WINCLOSE:
			DestroyWindow(winhandle);
			GuiEnd();
			return 1;
		case EV_KEY:
			if(!a1) break;
			switch(a0) {
			case 0x4B:
				key = KLEFT;
				break;
			case 0x72:
				key = KUP;
				break;
			case 0x77:
				key = KRIGHT;
				break;
			case 0x80:
				key = KDOWN;
				break;
			}
			struct message msg;
			msg.tid = game;
			msg.flags = 0;
			msg.send_size = 0;
			msg.recv_size = 0;
			send(&msg);
		}
	}
	
  return 0;
}

