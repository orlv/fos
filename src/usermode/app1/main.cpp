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
  int fd, fd1;
  while((fd = open("/dev/keyboard", 0)) == -1) sched_yield();
  printf("uptime=%d\n", uptime());
  char ch;
  char *buf;
  size_t len;
  extern int tty;
  
  while(1){
    if(read(fd, &ch, 1)){
      switch(ch){
      case 'u':
	printf("\nuptime=%d  ", uptime());
	break;

      case 'y':
	sched_yield();
	break;
	
      case 'a':
	fd1 = open("/dev/fda", 0);
	if(fd1 != -1 && tty) {
	  buf = new char[512];
	  read(fd1, buf, 512);
	  printf("\n--------------------------------------------------------------------------------");
	  write(tty, buf, 512);
	  printf("\n--------------------------------------------------------------------------------");
	  delete buf;
	}
	close(fd1);
	break;

      case 'd':
	if(tty) {
	  buf = new char[2048];
	  len = dmesg(buf, 2048);
	  write(tty, buf, len);
	  delete buf;
	}
	break;

	
      case 'g':
	fd1 = open("/mnt/modules/test.txt", 0);
	if(fd1 != -1 && tty) {
	  buf = new char[512];
	  read(fd1, buf, 512);
	  printf("\n--------------------------------------------------------------------------------");
	  write(tty, buf, 512);
	  printf("\n--------------------------------------------------------------------------------");
	  delete buf;
	}
	close(fd1);
	break;
	
      default:
	printf("%c", ch);
      }
    }
  }
  return 0;
}
