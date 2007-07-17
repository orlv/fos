/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos.h>
#include <string.h>
#include <stdio.h>
#include <fs.h>

#define FBTTY_CMD_SET_MODE (BASE_CMD_N + 0)
#define FBTTY_LOAD_FONT (BASE_CMD_N + 1)

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
  volatile struct message msg;
  
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

      case 'v':
	while(1){
	  fd1 = open("/dev/fbtty", 0);
	  if(fd1 != -1) {
	    msg.a0 = FBTTY_CMD_SET_MODE;
	    msg.a1 = 0x4114;
	    msg.send_size = 0;
	    msg.recv_size = 0;
	    msg.tid = ((fd_t) fd1)->thread;
	    send((message *)&msg);
	    close(fd1);
	    break;
	  }
	}
	break;

      case 'p':
	fd1 = open("/dev/fbtty", 0);
	if(fd1 != -1) {
	  msg.a0 = FBTTY_LOAD_FONT;
	  msg.send_size = strlen("/mnt/modules/font.psf") + 1;
	  msg.send_buf = "/mnt/modules/font.psf";
	  msg.recv_size = 0;
	  msg.tid = ((fd_t) fd1)->thread;
	  send((message *)&msg);
	  close(fd1);
	}
	break;
	
      default:
	printf("%c", ch);
      }
    }
  }
  return 0;
}
