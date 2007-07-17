/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos.h>
#include <stdio.h>
#include <fs.h>

asmlinkage int main()
{
  exec("tty");
  printf("Init started! If you see this text - all work fine.\n");
  exec("keyboard");
  exec("app1");
  exec("floppy");
  exec("fbtty");
  return 0;
}
