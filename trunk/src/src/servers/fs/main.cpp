/*
  Copyright (C) 2006-2007 Oleg Fedorov
*/

#include <fos.h>
#include <fs.h>
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
  printf("[TestFS]\n");
#if 0
  Tdirectory *dir;
  Tdirectory *fs;

  printf("Setting up ObjectFS && FSLayer..");
  dir = fs = new Tdirectory(new ObjectFS(0), 0);
  printf(" OK\n");

  dir->construct("dev", FTypeDirectory, 0);

  ls(dir);

  printf("TestFS: ready.");
  exec("app1");
#endif
  //Tinterface *object;
  struct message *msg = new struct message;
  
  u32_t res;
  struct fs_message *m = new fs_message;
  
  while(1){
    msg->recv_size = sizeof(fs_message);
    msg->recv_buf = m;
    receive(msg);
    printf("TestFS: cmd=%d, string=\"%s\"\n", m->cmd, m->name);

    switch(m->cmd){
    case FS_CMD_ACCESS:
      res = 666; //RES_FAULT;
      break;
      
    default:
      res = RES_FAULT;
    }
	
    msg->recv_size = 0;
    msg->send_size = sizeof(res);

    msg->send_buf = &res;
    reply(msg);
  }
}
