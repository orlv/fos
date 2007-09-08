/*
  startup.c
  Copyright (C) 2007 Oleg Fedorov
*/

#include <stdio.h>

int main(void);
void exit(void);

asm(".globl _start \n"
    "_start:       \n"
    "push %eax     \n"
    "call _startup");


void _startup(char *args)
{
  if(args)
    printf("args = [%s]", args);
    
  main();
  while(1);
  exit();
}
