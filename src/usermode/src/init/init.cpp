/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <stdio.h>
#include <stdlib.h>
#include <fos/fos.h>

asmlinkage int main()
{
  exec("tty");
  printf("Init started! If you see this text - all work fine.\n");
  exec("keyboard");
  exec("shell");
  //exec("floppy");
  exit(0);
  return 0;
}
