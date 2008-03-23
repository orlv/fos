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

nsi_object * nsi_t::find(char *name)
{
  if(!name)
    return &root;

  List <nsi_object *> *curr;
  list_for_each(curr, objects){
    if(!strcmp(name, curr->item->name))
      return curr->item;
  }
  return 0;
}

nsi_object * nsi_t::add(char *name)
{
  if(find(name))
    return 0;

  return objects->add_tail(new nsi_object(name))->item;
}

void nsi_t::remove(char *name)
{
  if(name) {
    List <nsi_object *> *curr, *n;
    list_for_each_safe(curr, n, objects){
      if(!strcmp(name, curr->item->name)){
	delete curr->item;
	delete curr;
      }
    }
  }
}

void nsi_t::wait_message()
{
  receive(msg);
  if(msg->arg[0] < MAX_METHODS_CNT) {

  }
}

//void nsi_t::wait_message(u32_t timeout);
