/*
  (C) 2008 Oleg Fedorov
*/

#include <fos/printk.h>
#include <fos/nsi.h>
#include <fos/fos.h>
#include <string.h>
#include <stdlib.h>

fos_nsi * nsi_init(char *bindpath)
{
  if(bindpath && bindpath[0])
    while(resmgr_attach(bindpath) != RES_SUCCESS);

  fos_nsi *nsi = malloc(sizeof(fos_nsi));
  nsi->msg = malloc(sizeof(struct message));
  return nsi;
}

int  nsi_add_method(fos_nsi *nsi, u32_t n, int (*method) (struct message *msg))
{
  if(n < MAX_METHODS_CNT){
    nsi->method[n] = method;
    return 0;
  }
  return 1;
}

void nsi_remove_method(fos_nsi *nsi, u32_t n)
{
  if(n < MAX_METHODS_CNT)
    nsi->method[n] = 0;
}

void nsi_wait_message(fos_nsi *nsi)
{
  /* сбрасываем параметры сообщения на стандартные */
  nsi->msg->send_buf = nsi->std.send_buf;
  nsi->msg->send_size = nsi->std.send_size;
  nsi->msg->recv_buf = nsi->std.recv_buf;
  nsi->msg->recv_size = nsi->std.recv_size;
  nsi->msg->pid = nsi->std.pid;
  nsi->msg->tid = nsi->std.tid;
  nsi->msg->flags = nsi->std.flags;

  /* получаем сообщение */
  receive(nsi->msg);

  /* если метод определён — вызываем его, иначе возвращаем ошибку */
  if((nsi->msg->arg[0] < MAX_METHODS_CNT) && nsi->method[nsi->msg->arg[0]]) {
    if(!nsi->method[nsi->msg->arg[0]](nsi->msg))
      return;
  } else {
    if(!nsi->method[0]){
      nsi->msg->arg[0] = 0;
      nsi->msg->arg[2] = ERR_UNKNOWN_METHOD;
    } else
      nsi->method[0](nsi->msg);
  }

  reply(nsi->msg);
}
