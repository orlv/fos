/*
  crt0.c
  Copyright (C) 2007-2008 Oleg Fedorov
                          Sergey Gridassov
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>

char **__ARGV = 0;
int __ARGC = 0;
char **environ;

FILE *stdout, *stdin, *stderr;

int errno = 0;

int daylight = 0;
long timezone = 0;

int main(int argc, char ** argv);

asm(".globl _start \n"
    "_start:       \n"
    "push %eax     \n"
    "push %ebx     \n"
    "push %ecx     \n"
    "push %edx     \n"
    "call _startup");

void _startup(int envp_size, char *envp, int args_size, char *args) 
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
  if(envp_size) {
    int envc = 0;
    for(char *ptr = envp;; envc++) {
      int len = strlen(ptr);
      if(!len) break;
      ptr += len + 1;
    }
    envc++;
    environ = (char **)malloc(envc * sizeof(char *));
    int i = 0;
    for(char *ptr = envp;; i++) {
      int len = strlen(ptr);
      if(!len) break;
      environ[i] = ptr;
      ptr += len + 1;
    }
  }else 
   environ = NULL;


  char *point = getenv("STDOUT");

  if(point)
    stdout = fopen(point, "w");
  else
    stdout = NULL;

  point = getenv("STDIN");
  if(point)
    stdin = fopen(point, "r");
  else
    stdin = NULL;

  point = getenv("STDERR");
  if(point)
    stderr = fopen(point, "w");
  else
    stderr = NULL;

  exit(main(argc, argv));
}
