#ifndef PRIVATETYPES_H
#define PRIVATETYPES_H
typedef struct {
	int w;
	int h;
	int class;
} create_win_t;
struct win_info {
	int bpp;
	int handle;
	int margin_up;
	int margin_down;
	int margin_left;
	int margin_right;
};
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

typedef struct {
	void * reserved[2];
	int class;
	int handle;
	int a0;
	int a1;
	int a2;
	int a3;
} event_t;

#define MAX_TITLE_LEN 64
#define WIN_CMD_CREATEWINDOW 1
#define WIN_CMD_DESTROYWINDOW 2
#define WIN_CMD_WAIT_EVENT 3
#define WIN_CMD_CLEANUP 4
#define WIN_CMD_MAPBUF 5
#define WIN_CMD_REFRESHWINDOW 6

#define RED(x, bits)	((x >> (16 + 8 - bits)) & ((1 << bits) - 1))
#define GREEN(x, bits)	((x >> (8 + 8 - bits)) & ((1 << bits) - 1))
#define BLUE(x, bits)	((x >> (8 - bits)) & ((1 << bits) - 1))
#endif
