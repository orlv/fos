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

  while(!exec("/mnt/modules/stty", NULL));
//  while(!exec("/mnt/modules/tty", NULL));	
  printf("Init started! If you see this text - all work fine.\n");
  exec("/mnt/modules/romfs", NULL);
  for(int i=0; i<50; i++) sched_yield();
  exec("/bin/i8042", NULL);
 exec("/usr/bin/pgs", NULL);
//  exec("/bin/shell", NULL);
  //exec("/mnt/modules/speaker", NULL);
  //exec("/mnt/modules/test2", NULL);
  //exec("/mnt/modules/pci_server", NULL);
  //exec("/mnt/modules/ext2test", "/mnt/modules/ext2.img");
  //exec("/mnt/modules/floppy");
  return 0;
}
