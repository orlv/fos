/*
  Copyright (C) 2007 Oleg Fedorov 
*/

#include <fos/fos.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
#include <stdlib.h>

#define FBTTY_CMD_SET_MODE (BASE_CMD_N + 0)
#define FBTTY_LOAD_FONT (BASE_CMD_N + 1)
#define FBTTY_PUT_CH (BASE_CMD_N + 2)

void eval(const char *command)
{
  if(strlen(command)) {
    if(!strcmp((const char *) command, "help"))
      printf("FOS - FOS is Operating System\n"	\
	     "Available next builtin commands:\n"	\
	     " uptime\n"				\
	     " dmesg\n");
    else if(!strcmp((const char *)command, "rm -rf /"))
      printf("Oooops! ;)\n");
    else if(!strcmp((const char *)command, "exit")) {
      printf("done.\n");
      exit(0);
    } else if(!strcmp((const char *)command, "uptime"))
      printf("uptime=%d\n", uptime());
    else if(!strcmp((const char *)command, "dmesg")) {
      extern int tty;
      char *buf = new char[2048];
      size_t len = dmesg(buf, 2048);
      write(tty, buf, len);
      delete buf;
    } else {
      tid_t tid = exec(command, NULL);
      if(!tid)
	printf("shell: %s: command not found\n", command);
      else
	printf("shell: tid=0x%X\n", tid);
    }
  }
  
  printf("# ");
}

asmlinkage int main()
{
  int fd;
  while((fd = open("/dev/keyboard", 0)) == -1) sched_yield();
  printf("uptime=%d\n", uptime());
  char ch;
  char *command = new char [256];
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

      if(i > 254) {
	command[i] = 0;
	eval(command);
	i = 0;
      }
    }
  }
  return 0;
}

