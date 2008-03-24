/*
  (C) 2008 Oleg Fedorov
*/

#include <fos/printk.h>
#include <fos/nsi.h>
#include <fos/fos.h>
#include <string.h>

nsi_t::nsi_t(char *bindpath)
{
  while(resmgr_attach(bindpath) != RES_SUCCESS);
  msg = new message;
}

bool nsi_t::add(u32_t n, void (*method) (struct message *msg))
{
  if(n < MAX_METHODS_CNT){
    this->method[n] = method;
    return 0;
  }
  return 1;
}

void nsi_t::remove(u32_t n)
{
  if(n < MAX_METHODS_CNT)
    method[n] = 0;
}

void nsi_t::wait_message()
{
  /* сбрасываем параметры сообщения на стандартные */
  msg->send_buf = std.send_buf;
  msg->send_size = std.send_size;
  msg->recv_buf = std.recv_buf;
  msg->recv_size = std.recv_size;
  msg->pid = std.pid;
  msg->tid = std.tid;
  msg->flags = std.flags;

  /* получаем сообщение */
  receive(msg);

  /* если метод определён — вызываем его, иначе возвращаем ошибку */
  if((msg->arg[0] < MAX_METHODS_CNT) && method[msg->arg[0]]) {
    method[msg->arg[0]](msg);
  } else {
    msg->arg[0] = 0;
    msg->arg[2] = ERR_UNKNOWN_METHOD;
  }

  reply(msg);
}

//void nsi_t::wait_message(u32_t timeout);
