/*
  Copyright (C) 2007 Oleg Fedorov
*/

#include <fos.h>
#include <string.h>
#include <stdio.h>

#include "tinterface.h"
#include "fs.h"
#include "fslayer.h"
#include "fs/objectfs/objectfs.h"

static inline void ls(Tdirectory * dir)
{
  obj_info_t *dirent;
  off_t i;
  for (i = 0; (dirent = dir->list(i)); i++) {
    if (dirent->info.type == FTypeDirectory)
      printf("drwxrwxrwx 1 root:root %6d %s\n", dirent->info.size,
	     dirent->name);
    else
      printf("-rwxrwxrwx 1 root:root %6d %s\n", dirent->info.size,
	     dirent->name);

    delete dirent;
  }
}

asmlinkage void _start()
{
  printf("[FS]\n");

  Tdirectory *dir;
  Tdirectory *fs;
  Tobject *obj;

  printf("Setting up ObjectFS && FSLayer..");
  dir = fs = new Tdirectory(new ObjectFS(0), 0);
  printf(" OK\n");

  dir->construct("dev", FTypeDirectory, 0);

  ls(dir);

  printf("FS: ready.");

  char *buf = new char[256];
  struct msg *msg = new struct msg;
  strcpy(buf, "app1");
  int i = strlen(buf);
  buf[i] = 0;
  msg->send_buf = msg->recv_buf = buf;
  msg->send_size = i + 1;
  msg->recv_size = 10;
  msg->pid = 0;
  send(msg);
  
  printf("\nmsg send.");
  while (1);
}
