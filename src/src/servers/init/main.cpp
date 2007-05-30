/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos.h>
#include <stdio.h>
#include <fs.h>

asmlinkage void _start()
{
  exec("tty"); /* pid = 4 */
  printf("Init started\n");
  exec("keyboard"); /* pid = 5 */
  exec("app1"); /* pid = 6 */
  while(1);
}
