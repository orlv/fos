/*
  Copyright (C) 2007 Oleg Fedorov 
*/

#include <fos.h>
#include <string.h>
#include <stdio.h>
#include <fs.h>

#define FBTTY_CMD_SET_MODE (BASE_CMD_N + 0)
#define FBTTY_LOAD_FONT (BASE_CMD_N + 1)
#define FBTTY_PUT_CH (BASE_CMD_N + 2)

void eval(const unsigned char *command)
{
  //printf("%s\n", command);
  if(!strcmp((const char *) command, "help"))
    printf("FOS - FOS is Operating System\n"	\
	   "Available next builtin commands:\n" \
	   " uptime\n"				\
	   " dmesg\n");

  if(!strcmp((const char *)command, "rm -rf /"))
    printf("Oooops! ;)\n");

  if(!strcmp((const char *)command, "uptime"))
    printf("uptime=%d\n", uptime());

  if(!strcmp((const char *)command, "dmesg")) {
    extern int tty;
    char *buf = new char[2048];
    size_t len = dmesg(buf, 2048);
    write(tty, buf, len);
    delete buf;
  }

  if(!strcmp((const char *)command, "test"))
    printf("executing \"test\", tid=0x%X\n", exec("test"));
  
  printf("# ");
}

asmlinkage int main()
{
  int fd;
  while((fd = open("/dev/keyboard", 0)) == -1) sched_yield();
  printf("uptime=%d\n", uptime());
  char ch;
  unsigned char *command = new unsigned char [256];
  size_t i=0;
  printf("\nWelcome to FOS Operating System\n" \
	 "# ");
  while(1){
    if(read(fd, &ch, 1)){
      switch(ch){
      case '\n':
	printf("%c", ch);
	command[i] = 0;
	eval(command);
	i=0;
	command[0] = 0;
	break;

      case 0x08: /* backspace */
	if(i){
	  printf("%c", ch);
	  command[i] = 0;
	  i--;
	}
	break;
	
      default:
	printf("%c", ch);
	command[i] = ch;
	i++;
      }

      if(i > 254)
	eval(command);

#if 0
      switch(ch){
      case 'a':
	fd1 = open("/dev/fda", 0);
	if(fd1 != -1 && tty) {
	  buf = new char[512];
	  read(fd1, buf, 512);
	  printf("\n-------------------------------------------------------------------------------\n");
	  write(tty, buf, 512);
	  printf("\n-------------------------------------------------------------------------------\n");
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
	  printf("\n-------------------------------------------------------------------------------\n");
	  write(tty, buf, 512);
	  printf("\n-------------------------------------------------------------------------------\n");
	  delete buf;
	}
	close(fd1);
	break;
#if 0
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

      case '.':
	while(1){
	  fd1 = open("/dev/fbtty", 0);
	  if(fd1 != -1) {
	    msg.a0 = FBTTY_PUT_CH;
	    msg.a1 = '.';
	    msg.send_size = 0;
	    msg.recv_size = 0;
	    msg.tid = ((fd_t) fd1)->thread;
	    send((message *)&msg);
	    close(fd1);
	    break;
	  }
	}
	break;
#endif	
      default:
	printf("%c", ch);
      }
#endif
    }
  }
  return 0;
}

