#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fgs/controls.h>
#include <fgs/fgs.h>
int hndl;
int EventHandler(int hwnd, int class, int a0, int a1, int a2, int a3)
{
	DestroyControlsWindow(hndl);
	return 0;
}
int main(int argc, char *argv[]) {
	if(argc < 2) {
		printf("Usage: %s <message>\n", argv[0]);
		return 1;
	}
	char *buf = NULL;
	int size = 0;
	for(int i = 1; i < argc; i++) {
		size += strlen(argv[i]) + 1;
		buf = realloc(buf, size + 1);
		strcat(buf, argv[i]);
		strcat(buf, " ");
	}
	GUIInit();
	int width, height;
	ScreenInfo(&width, &height);
	int maxlines = (height / 4) / 16;
	int maxsymbols = (width / 4) / 8;
	height = strlen(buf) / maxsymbols;
	height++;
	hndl = CreateControlsWindow(0, 0, maxsymbols * 8 + 10, height * 16 + 10 + 5 + 25, "Message", EventHandler, WC_CENTERED);
	int currentline = 0;
	for(char *ptr = buf; currentline < maxlines;currentline++) {
		int part = strlen(ptr);
		if(!part) break;
		if(part > maxsymbols) 
			ptr[maxsymbols] = 0;

		CreateStatic(hndl, 5, currentline * 16 + 5, maxsymbols * 8, 16, ptr);
		if(part > maxsymbols) 
			ptr +=maxsymbols + 1;
		else
			ptr +=part;
	}
	CreateButton(hndl, (maxsymbols * 8 - 42) / 2, height * 16 + 5, 42, 25, "OK");
	ControlsWindowVisible(hndl, 1);
	ControlsMessageLoop();
	GuiEnd();
	return 0;
}
 
