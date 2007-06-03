/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos.h>
#include <stdio.h>
#include <fs.h>

extern tid_t procman;
extern tid_t namer;
extern tid_t tty;

asmlinkage void _start()
{
  while(!(namer = resolve("/sys/namer")));
  while(!(procman = resolve("/sys/procman")));

  exec("tty");

  while(!(tty = resolve("/dev/tty")));
  printf("Init started! If you see this text - all work fine.\n");
#if 0

  printf("Init started\n");
  exec("keyboard");
  exec("app1");
#endif
  while(1);
}
