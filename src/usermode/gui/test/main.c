/*
 * Copyright (c) 2007 - 2008 Sergey Gridassov
 */

#include <stdio.h>
#include <string.h>
#include <fgs/fgs.h>
#include <fgs/controls.h>

int button[3];
int hndl;
int st;

int EventHandler(int hwnd, int class, int a0, int a1, int a2, int a3)
{
  switch (class) {
  case EV_WINCLOSE:
    DestroyControlsWindow(hwnd);
    return 0;
  case EVC_CLICK:
    if (hwnd == button[0]) {
      SetControlText(st, "Clicked!");
      SetControlText(button[1], "Exit :(");
      return 1;
    } else if (hwnd == button[1]) {
      DestroyControlsWindow(hndl);
      return 0;
    } else if (hwnd == button[2]) {
      DestroyControl(st);
      DestroyControl(button[2]);
      DestroyControl(button[0]);
      return 1;
    }
  }
  return 1;

}

asmlinkage int main(int argc, char **argv)
{
  GUIInit();
  hndl = CreateControlsWindow(0, 0, 280, 70, "Hello, World", EventHandler, WC_WINDOW);
  button[0] = CreateButton(hndl, 10, 10, 90, 25, "Click me!");
  button[1] = CreateButton(hndl, 110, 10, 70, 25, "Exit");
  button[2] = CreateButton(hndl, 190, 10, 80, 25, "Destroy");
  st = CreateStatic(hndl, 10, 40, 260, 16, "Waiting..");
  ControlsWindowVisible(hndl, 1);
  ControlsMessageLoop();
  GuiEnd();
  return 0;
}
