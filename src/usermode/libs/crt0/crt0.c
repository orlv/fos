/*
  startup.c
  Copyright (C) 2007 Oleg Fedorov
*/

#include <stdio.h>
#include <stdlib.h>

char **__ARGV = 0;
int __ARGC = 0;
char **environ;

int main(int argc, char ** argv);

asm(".globl _start \n"
    "_start:       \n"
    "push %eax     \n"
    "push %ebx     \n"
    "push %ecx     \n"
    "push %edx     \n"
    "call _startup");

void _startup(int envp_size, char **envp, int args_size, char *args) 
{

  int argc = 0;
  char **argv = NULL;
  char *p = args;
  if(args_size) {
    while(*p) {
      while(*p == ' ') p++;
      if(*p) argc++;
      while(*p && (*p != ' ')) p++;
    }
    argv = (char **) malloc(sizeof(char *) * argc);
    p = args;
    int i;
    for(i=0; *p; p++, i++) {
      while(*p == ' ') { *p=0; p++; }
      if(*p) argv[i] = p;
      while(*p && (*p != ' ')) p++;
      if(*p == ' ') *p=0;
    }
  }
  __ARGC = argc;
  __ARGV = argv;
  environ = envp;
  if(!environ || !envp_size) {
   environ = malloc(1);
   environ[0] = 0;
  }
  exit(main(argc, argv));
}
