/*
  startup.c
  Copyright (C) 2007 Oleg Fedorov
*/

int main(void);

#include <fos.h>

tid_t namer;
tid_t tty;
tid_t procman;

void _start()
{
  while(!(namer = resolve("/sys/namer")));
  while(!(procman = resolve("/sys/procman")));
  tty = 0;

  main();
  while(1);
}
