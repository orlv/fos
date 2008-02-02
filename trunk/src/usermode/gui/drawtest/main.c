/*
 * Copyright (c) 2008 Sergey Gridassov
 */

#define CNT 512	/* количество итераций. больше - точнее, но дольше выполняется */

#include <stdio.h>
#include <string.h>
#include <fgs/fgs.h>
#include <fos/fos.h>
#include <fgs/controls.h>



int EventHandler(int hwnd, int class, int a0, int a1, int a2, int a3)
{
  switch (class) {
  case EV_WINCLOSE:
    DestroyControlsWindow(hwnd);
    return 0;
  }
  return 1;

}

asmlinkage int main(int argc, char **argv)
{
  char *rot[] = { "|", "/", "-", "\\" };
  GUIInit();
  int hndl = CreateControlsWindow(0, 0, 280, 70, "Drawing speed", EventHandler, WC_WINDOW);
  int st;
  st = CreateStatic(hndl, 10, 10, 260, 16, "drawing speed test");
  int st2 = CreateStatic(hndl, 10, 30, 260, 16, "please wait");
  int st3 = CreateStatic(hndl, 0, 0, 8, 16, ".");
  ControlsWindowVisible(hndl, 1);
  int start = uptime();
  for(int i = 0; i < CNT - 1; i++) {
   SetControlText(st3, rot[i % 4]);
  }
  SetControlText(st3, " ");
  int total = uptime() - start;
  char tmp[256];
  snprintf(tmp, 256, "refresh taking %u ms", total / CNT);
  SetControlText(st2, tmp);
  ControlsMessageLoop();
  GuiEnd();
  return 0;
}
