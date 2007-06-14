/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos.h>
#include <string.h>
#include <stdio.h>
#include <fs.h>

asmlinkage int main()
{
  printf("{Hello app1}\n");

  fd_t fd;
  while(!(fd = open("/dev/keyboard", 0)));

  char ch;
  while(1){
    read(fd, &ch, 1);
    printf("%c", ch);
  }
    
  return 0;
}
