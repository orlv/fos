/*
  namer/namer.cpp
  Copyright (C) 2006-2007 Oleg Fedorov

  - (Fri Jul 13 17:51:58 2007) перемещено в usermode
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

#include <fos/fs.h>
#include <fos/fos.h>
#include <fos/message.h>
#include <fos/namer.h>
#include <string.h>
#include <stdio.h>
#include "namer.h"

asmlinkage int main()
{
  tree *namer = new tree;
  branch *obj;
  message *msg = new message;
  char *pathname = new char[MAX_PATH_LEN];

  while (1) {
    msg->recv_size = MAX_PATH_LEN;
    msg->recv_buf = pathname;
    msg->tid = 0;
    msg->flags = 0;
    receive(msg);
    switch (msg->arg[0]) {
    case NAMER_CMD_ADD:
      //printf("namer: adding [%s]\n", pathname);
      obj = namer->add(pathname, (void *)msg->tid);
      msg->arg[0] = RES_SUCCESS;
      msg->send_size = 0;
      reply(msg);
      break;

    case FS_CMD_UNLINK:
    case FS_CMD_ACCESS:
    case FS_CMD_DIROPEN:
    case FS_CMD_MKDIR:
    case FS_CMD_RMDIR:
    case FS_CMD_POSIX_ACCESS:
      //printf("namer: requested access to [%s]\n", pathname);
      obj = namer->find_branch_last_match(pathname);
      //printf("[0x%X]", obj);
      if(obj->data) {
	//printf("namer: access granted [%s]\n", pathname);
	msg->send_size = strlen(pathname);
	msg->send_buf = pathname;
	if (forward(msg, (tid_t)obj->data) != RES_SUCCESS) {
	  msg->arg[0] = 0;
	  msg->arg[2] = ERR_NO_SUCH_FILE;
	  reply(msg);
	}
      } else {
	//printf("namer: access denied\n");
	msg->send_size = 0;
	msg->arg[0] = 0;
	msg->arg[2] = ERR_NO_SUCH_FILE;
	reply(msg);
      }
      break;

    case FS_CMD_STAT:
      obj = namer->find_branch_last_match(pathname);
      if (obj->data) {
	msg->send_size = strlen(pathname);
	msg->send_buf = pathname;
	if (forward(msg, (tid_t)obj->data) != RES_SUCCESS) {
	  msg->arg[0] = 0;
	  msg->arg[2] = ERR_NO_SUCH_FILE;
	  reply(msg);
	}
      } else {
	msg->send_size = 0;
	msg->arg[0] = 0;
	msg->arg[2] = ERR_NO_SUCH_FILE;
	reply(msg);
      }
      break;

    case NAMER_CMD_RESOLVE:{
      //printf("namer: resolving [%s]\n", pathname);
      obj = namer->find_branch_last_match(pathname);
      if (obj->data) {
	msg->send_size = msg->arg[0] = strlen(pathname);
	msg->send_buf = pathname;
	msg->arg[1] = (tid_t)obj->data;
	msg->arg[2] = NO_ERR;
      } else {
	msg->send_size = 0;
	msg->arg[1] = 0;
	msg->arg[2] = ERR_NO_SUCH_FILE;
      }
      reply(msg);
      break;
    }

    default:
      msg->send_size = 0;
      msg->arg[0] = 0;
      reply(msg);
    }
  }
  return 0;
}
