/*
 *         FOS Graphics System
 * Copyright (c) 2007 Sergey Gridassov
 */


#include <stdio.h>
#include <fos/fos.h>
#include <sched.h>
#include <stdlib.h>
#include <private/types.h>
#include <private/loadfile.h>
#include <private/vbe.h>
#include <private/windowing.h>
#include <private/cursor.h>
#include <private/events.h>

int need_cursor = 0;
picture_t *busy, *cursor, *close_button;
extern context_t *backbuf;
context_t screen;
extern char *font;

int main(int argc, char *argv)
{
  printf("FOS Graphics System version " VERSION " started up\n");
  srandom(uptime());
  cursor = load_file("/usr/share/cursors/cursor.pct");
  busy = load_file("/usr/share/cursors/busy.pct");
  close_button = load_file("/usr/share/pixmaps/close.pct");
  font = load_file("/usr/share/fonts/font.psf");
  if (!cursor || !busy || !close_button || !font) {
    printf("FGS: Error loading some graphics file\n");
    return 1;
  }
  printf("FGS: resources loaded (4 file ok)\n");
  if(!init_video(800, 600, 16, &screen)) {
    printf("FGS: Setting mode failed.\n");
    return 1;
  }
  printf("FGS: Mode set: %ux%ux%u\n", screen.w, screen.h, screen.bpp * 8);
  mousex = screen.w / 2;
  mousey = screen.h / 2;
  init_windowing();
  StartEventHandling();
  SetBusy(-1);
  exec("/usr/bin/taskbar", NULL);
  printf("FGS: Started up shell (/usr/bin/taskbar)\n");
  for (;;) {
    if (need_refresh) {
      Redraw();
      need_refresh = 0;
    }
    if (need_cursor) {
      RedrawCursor();
      need_cursor = 0;
    }

  }
  return 0;
}
