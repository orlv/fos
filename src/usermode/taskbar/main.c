#include <stdio.h>
#include <string.h>
#include <pgs/pgs.h>
#include <pgs/controls.h>
int EventHandler(int hwnd, int class, int a0, int a1, int a2, int a3) {
	switch(class) {
	case EV_WINCLOSE:
		DestroyControlsWindow(hwnd);
		return 0;
	}

	return 1;

}
asmlinkage int main(int argc, char ** argv)
{
        GUIInit();
	int width, height;
	ScreenInfo(&width, &height);
	int hndl = CreateControlsWindow(0, height - 28, width, 28, "TaskBar", EventHandler, WC_NODECORATIONS);
	int drawing = GetDrawingHandle(hndl);
	line(drawing, 0, 0, width, 0, 0xD8D8D8);
	line(drawing, 0, 1, width, 1, 0xF8F8F8);

	int button = CreateButton(hndl, 3, 4, 56, 22, "Start");
	ControlsWindowVisible(hndl, 1);
	ControlsMessageLoop();
        GuiEnd();
	return 0;
}
