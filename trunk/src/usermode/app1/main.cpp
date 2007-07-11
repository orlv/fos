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
  while(!(fd = open("/dev/keyboard", 0))) sched_yield();

  printf("uptime=%d\n", uptime());
  char ch;
  while(1){
    if(read(fd, &ch, 1)){
      switch(ch){
      case 'u':
	printf("uptime=%d\n", uptime());
	break;
	
      default:
	printf("%c", ch);
      }
    }
  }
  return 0;
}
