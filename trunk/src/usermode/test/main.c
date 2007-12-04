#include <stdio.h>
#include <string.h>
#include "gui.h"
asmlinkage int main(int argc, char ** argv)
{
        GUIInit();
        int winhandle = CreateWindow(220, 64, "Fonts", WC_WINDOW);
	pstring(winhandle, 8, 8, 0x000000, "Hello, World!");
	pstring(winhandle, 8, 16 + 8, 0x00007F, "This is microkernel GUI!");
	RefreshWindow(winhandle);
        int class, handle, a0, a1, a2, a3;
        while(1) {
                WaitEvent(&class, &handle, &a0, &a1, &a2, &a3);
                switch(class) {
                case  EV_WINCLOSE:
                        DestroyWindow(winhandle);
                        GuiEnd();
                        return 1;
                case EV_MDOWN:
                        break;
                case EV_MUP:
                        break;
                }
        }
        
 return 0;
}
