#include <stdio.h>
#include <string.h>
#include <pgs/pgs.h>
#include <pgs/controls.h>
#define ITEMS_COUNT 2
static char *items[ITEMS_COUNT] = { "Tetris", "Test" };
static char *cmds[ITEMS_COUNT] = { "/usr/bin/tetris", "/bin/test" };
int width, height;
int hndl;
int displayed = 0;
		int menu;
int EventHandler(int hwnd, int class, int a0, int a1, int a2, int a3) {
	switch(class) {
	case EV_WINCLOSE:
		DestroyControlsWindow(hwnd);
		return 0;
	case EVC_CLICK: {
		if(!displayed) {
			menu = CreateMenu(hndl, 0, height - 28, ITEMS_COUNT, items); // hndl, 3
			displayed = 1;
		} else {
			DestroyControl(menu);
			displayed = 0;
		}
		return 1;
	}
	case EVC_MENU:
		DestroyControl(hwnd);
		displayed = 0;
		exec(cmds[a0], NULL);
		return 1;
	}
	return 1;

}
asmlinkage int main(int argc, char ** argv)
{
        GUIInit();
	ScreenInfo(&width, &height);
	hndl = CreateControlsWindow(0, height - 28, width, 28, "TaskBar", EventHandler, WC_NODECORATIONS);
	int drawing = GetDrawingHandle(hndl);
	line(drawing, 0, 0, width, 0, 0xD8D8D8);
	line(drawing, 0, 1, width, 1, 0xF8F8F8);
	int button;
	button = CreateButton(hndl, 3, 4, 56, 22, "Start");
	ControlsWindowVisible(hndl, 1);
	ControlsMessageLoop();
        GuiEnd();
	return 0;
}
