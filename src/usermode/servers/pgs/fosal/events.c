#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <fos/fos.h>
#include <sched.h>

#include <gui/types.h>
void event_handler(event_t *event);
struct mouse_pos {
	int dx;
	int dy;
	int dz;
	int b;
};
extern mode_definition_t __current_mode;
void mouse_thread() {
	int x = 0;
	int y = 0;
	int oldx = 0;
	int oldy = 0;
	int oldb = 0;
	int psaux;
	do {
		psaux = open("/dev/psaux", 0);
		sched_yield();
	} while(!psaux);
	struct mouse_pos move;
	while(1) {
		read(psaux, &move, sizeof(struct mouse_pos));
		x += move.dx;
		y += move.dy;
		if(x < 0) x = 0;
		if(y < 0) y = 0;
		if(x > __current_mode.width) x = __current_mode.width;
		if(y > __current_mode.height) y = __current_mode.height;
		if(x != oldx || y != oldy) { // мышь сдвинули
			oldx = x;
			oldy = y;
			mousemove_event_t mouse = { x, y};
			event_t event = { EVENT_TYPE_MOUSEMOVE, &mouse };
			event_handler(&event);
		}
		if(move.b && !oldb) { // кнопку нажали
			event_t event = { EVENT_TYPE_MOUSEDOWN, NULL };
			event_handler(&event);
			oldb = move.b;
		}
		if(!move.b && oldb) { // отпустили
			event_t event = { EVENT_TYPE_MOUSEUP, NULL };
			event_handler(&event);
			oldb = move.b;	
		}
			
	}
} 

void StartEventHandling() {
	thread_create(mouse_thread);
}
