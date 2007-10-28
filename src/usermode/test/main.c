#include <stdio.h>
#include "gui.h"
asmlinkage int main(int argc, char ** argv)
{
	GUIInit();
	int winhandle = CreateWindow(0, 10, 10, 400, 50, "4 окна друг на друге :)", WC_WINDOW);
	int class, handle, a0, a1, a2, a3;
	while(1) {
		WaitEvent(&class, &handle, &a0, &a1, &a2, &a3);
		switch(class) {
		case  EV_WINCLOSE:
			DestroyWindow(winhandle);
			GuiEnd();
			return 1;
			break;
		}
	}
	
  return 0;
}

