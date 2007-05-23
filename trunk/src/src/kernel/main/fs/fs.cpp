/*
  kernel/main/fs/fs.cpp
  Copyright (C) 2006-2007 Oleg Fedorov
*/

/*
  Namer играет вспомогательную роль при определении сервера, отвечающего
  за конечный файл.

  Допустим, при получении запроса на открытие файла /tmp/foo.txt
  и смонированном на каталог /tmp сервера с pid=34, запрос модифицируется
  на /foo.txt и переадресуется данному серверу.

  Далее сервер сам решает, позволять ли доступ к файлу. Сервер не знает
  об факте переадресации, она производится прозрачно.
  reply от сервера уходит к вызвавшему приложению.

  Для открытия файлов следует пользоваться namer (иначе в случае вложенного монтирования
  будет открыт не тот файл)
 */

#include <hal.h>
#include <string.h>
#include <stdio.h>
#include <fs.h>
#include <fslayer.h>
#include <drivers/fs/objectfs/objectfs.h>
#include <system.h>

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

void namer()
{
  printk("[Namer]\n");

  //  Tdirectory *dir;
  //  Tdirectory *fs;

  //  printf("Setting up ObjectFS && FSLayer..");
  //  dir = fs = new Tdirectory(new ObjectFS(0), 0);
  //  printf(" OK\n");

  //  dir->construct("dev", FTypeDirectory, 0);

  //  ls(dir);

  //printf("FS: ready.");
  // exec("app1");

  //Tinterface *object;
  struct message *msg = new struct message;
  struct fs_message *m = new fs_message;
  u32_t res;


  while(1){
    msg->recv_size = sizeof(fs_message);
    msg->recv_buf = m;
    receive(msg);
    printk("Namer: cmd=%d, string=\"%s\"\n", m->cmd, m->name);
      
    switch(m->cmd){
    case NAMER_CMD_ADD:
      printk("Namer: adding %d as \"%s\"", msg->pid, m->name);
      while(1);
      res = RES_SUCCESS;
      msg->recv_size = 0;
      msg->send_size = sizeof(res);
      msg->send_buf = &res;
      reply(msg);
      break;

    case NAMER_CMD_REM:
    case NAMER_CMD_LIST:
    case NAMER_CMD_ACCESS:
    default:
      res = RES_FAULT;
      msg->recv_size = 0;
      msg->send_size = sizeof(res);
      msg->send_buf = &res;
      reply(msg);
    }

    //forward(msg, 4);  
  }
}
