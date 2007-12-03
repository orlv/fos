#include <stdio.h>
#include <string.h>
#include "gui.h"
asmlinkage int main(int argc, char ** argv)
{
        GUIInit();

        int winhandle = CreateWindow(400, 70, "Hello World", WC_WINDOW);
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
