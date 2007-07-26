/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <stdio.h>
#include <stdlib.h>
#include <fos/fos.h>
#include <sched.h>

asmlinkage int main()
{
  for(int i=0; i<15; i++){
    sched_yield();
  }

  while(!exec("/mnt/modules/tty"));
  printf("Init started! If you see this text - all work fine.\n");
  exec("/mnt/modules/keyboard");
  exec("/mnt/modules/shell");
  //exec("floppy");
  return 0;
}
