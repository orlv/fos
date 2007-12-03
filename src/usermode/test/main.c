#include <stdio.h>
#include <string.h>
#include "gui.h"
asmlinkage int main(int argc, char ** argv)
{
	int i;
	GUIInit();
	char title[128];
	strcpy(title, "Window ");
	if(argc>1)
		for(i=1; i<argc; i++)
			strcat(title, argv[i]);

	int winhandle = CreateWindow(400,70, title, WC_WINDOW);
	int class, handle, a0, a1, a2, a3;
	while(1) {
		WaitEvent(&class, &handle, &a0, &a1, &a2, &a3);
		switch(class) {
		case  EV_WINCLOSE:
			DestroyWindow(winhandle);
			GuiEnd();
			return 1;
		case EV_MDOWN:
			pixel(winhandle, a0, a1, 0xFF0000);
			RefreshWindow(winhandle);
			break;
		case EV_MUP:
			break;			
		}
	}
	
  return 0;
}

