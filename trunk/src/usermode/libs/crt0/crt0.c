/*
  startup.c
  Copyright (C) 2007 Oleg Fedorov
*/

#include <stdio.h>
#include <stdlib.h>

char **__ARGV = 0;
int __ARGC = 0;
char **__ENVP = 0;

int main(int argc, char ** argv);

asm(".globl _start \n"
    "_start:       \n"
    "push %eax     \n"
    "push %ebx     \n"
    "call _startup");

void _startup(char *args, char *evnp)
{
  int argc = 0;
  char **argv;
  char *p = args;

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

  __ARGC = argc;
  __ARGV = argv;

  exit(main(argc, argv));
}
