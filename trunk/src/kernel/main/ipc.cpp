/*
 * kernel/main/ipc.cpp
 * Copyright (C) 2005-2008 Oleg Fedorov
 */

#include <fos/printk.h>
#include <fos/fos.h>
#include <fos/syscall.h>
#include <fos/pager.h>
#include <fos/drivers/char/timer/timer.h>
#include <string.h>

List<kmessage *> * Messenger::msg_list::get(Thread *sender, u32_t flags)
{
  List<kmessage *> *entry;
  /* пройдём по списку в поисках нужного сообщения */
  list_for_each (entry, list) {
    if(entry->item->thread == sender)
      return entry;
  }
  /* сообщение не найдено, и не придёт */
  if(sender && count.value() > MAX_MSG_COUNT)
    return -1;
      
  /* сообщение не найдено, но можно повторить попытку позже */
  return 0;
}

/* находит сообщение в списке непрочтённых */
kmessage *Messenger::get(Thread *sender, u32_t flags)
{
  kmessage *msg;
  List<kmessage *> *entry;

  if((from) || (flags & MSG_ASYNC)) {
    entry = unread.get(sender, flags);
  } else { /* любое сообщение */
    entry = (unread.count.value())?(unread.list.next):(0);
  }

  if(!entry || (entry == -1)) return (kmessage *)entry; /* возврат ошибки */

  msg = entry->item;
  if (!(msg->flags & MSG_ASYNC)){ /* перемещаем сообщение в очередь полученных сообщений */
    entry->move_tail(&read.list);
    read.count.inc();
  } else /* убираем сообщение из списков */
    delete entry; 

  unread.count.dec();
  return msg;
}

#define MSG_CHK_SENDBUF  1
#define MSG_CHK_RECVBUF  2
#define MSG_CHK_FLAGS    4

static void kill_message(kmessage *message)
{
  message->reply_size = 0;
  message->size = 0;
  Thread *thread = message->thread;
  if(!(message->flags & MSG_ASYNC))
    thread->start(WFLAG_SEND);
    //thread->flags &= ~FLAG_TSK_SEND; /* сбросим у отправителя флаг TSK_SEND */
}

static inline bool check_message(message *message, int flags)
{
  if(OFFSET(message) < system->procman->current_thread->process->memory->mem_base) {
    printk("kernel: message check base failed (0x%X < 0x%X)\n",
	   OFFSET(message), system->procman->current_thread->process->memory->mem_base);
    return 1;
  }
  
  u32_t *pagedir = system->procman->current_thread->process->memory->pager->pagedir;
  u32_t count;

  count = (OFFSET(message)%PAGE_SIZE + sizeof(struct message) + PAGE_SIZE - 1)/PAGE_SIZE;

  /* страницы должны быть присоединены в адресное пространство процесса */
  if(!check_pages(PAGE(OFFSET(message)), pagedir, count)){
    printk("kernel: check_pages() on message failed\n");
    return 1;
  }

  /* проверим присутствие страниц буфера */
  if((flags & MSG_CHK_RECVBUF) && message->recv_size && !(message->flags & (MSG_MEM_SEND | MSG_MEM_SHARE))) {
    count = (OFFSET(message->recv_buf)%PAGE_SIZE + message->recv_size + PAGE_SIZE - 1)/PAGE_SIZE;
    if(!check_pages(PAGE(OFFSET(message->recv_buf)), pagedir, count)) {
      printk("kernel: check_pages() on message->recv_buf failed \n\
kernel: recv_buf=0x%X, pd=0x%X, count=0x%X\n", message->recv_buf, pagedir, count);
      return 1;
    }
  }

  if((flags & MSG_CHK_SENDBUF) && message->send_size) {
    count = (OFFSET(message->send_buf)%PAGE_SIZE + message->send_size + PAGE_SIZE - 1)/PAGE_SIZE;
    if(!check_pages(PAGE(OFFSET(message->send_buf)), pagedir, count)){
      printk("kernel: check_pages() on message->send_buf failed \n\
kernel: send_buf=0x%X, pd=0x%X, count=0x%X\n", OFFSET(message->send_buf), pagedir, count);
      return 1;
    }
  }

  if(message->flags & MSG_ASYNC) /* процесс не может устанавливать флаг ASYNC */
    message->flags &= ~MSG_ASYNC; 

  /* если используется разделение или передача разделяемых страниц - проверяем выравнивание */
  if((flags & MSG_CHK_FLAGS) && (message->flags & (MSG_MEM_SEND | MSG_MEM_SHARE)) && (OFFSET(message->send_buf) & 0xfff)) {
    printk("kernel: SHM buffer not aligned!\n\
kernel: message->send_buf=0x%X\n", OFFSET(message->send_buf));
    return 1;
  }

  return 0;
}


void kmessage::init(u32_t flags, message *msg)
{
  /* проверим флаги */

  if(flags & MSG_MEM_TAKE) {
    if(!(this->flags & MSG_MEM_SEND)){
      size = 0; /* при несовпадении флагов не передаем буфер, передаем только аргументы */
    }
  } else if(flags & MSG_MEM_SHARE) {
    if(!(this->flags & MSG_MEM_SHARE)){
      size = 0;
    }
  } else if((this->flags & (MSG_MEM_SEND | MSG_MEM_SHARE))) {
    size = 0;
  }

  msg->send_size = reply_size;
  
  /* проверим соответствие размеров буферов
     решаем, сколько байт сообщения копировать */
  
  if(size > msg->recv_size)
    size = msg->recv_size;
  else
    msg->recv_size = size;
}

void Messenger::import(kmessage *kmsg, message *msg)
{
  if (kmsg->size) {
    size_t size = kmsg->size;
    if(!(kmsg->flags & (MSG_MEM_SEND | MSG_MEM_SHARE))) {
      memcpy(msg->recv_buf, kmsg->buffer, rcv_size); /* копируем сообщение из ядра в память получателя */
      delete (u32_t *) kmsg->buffer; /* переданные данные больше не нужны в ядре, освобождаем память */
    } else {
      size &= ~0xfff;
      if(size) {
	/* монтируем буфер в свободное место адр. пр-ва получателя */
	msg->recv_buf = thread->process->memory->mmap(0, size, 0, OFFSET(kmsg->buffer), kmsg->thread->process->memory);
	
	if(kmsg->flags & MSG_MEM_SEND) /* демонтируем буфер из памяти отправителя */
	  kmsg->thread->process->memory->munmap(OFFSET(kmsg->buffer), kmsg->size);
      }
    }
  }

  msg->arg[0] = kmsg->arg[0];
  msg->arg[1] = kmsg->arg[1];
  msg->arg[2] = kmsg->arg[2];
  msg->arg[3] = kmsg->arg[3];

  if((kmsg->flags & MSG_ASYNC)) { 
    msg->tid = 0;
    msg->pid = 0;
    msg->flags &= MSG_ASYNC;
    delete kmsg; /* асинхронные сообщения не требуют ответа, сразу удаляем из ядра */
  } else {
    msg->tid = kmsg->thread->tid; /* укажем отправителя сообщения */
    msg->pid = kmsg->thread->process->pid;
  }
}

res_t receive(message *msg)
{
  if(check_message(msg, MSG_CHK_RECVBUF))
    return RES_FAULT;

  kmessage *kmsg = 0;
  Thread *me = system->procman->current_thread;
  Thread *sender = (msg->tid)?(THREAD(msg->tid)):(0);
  
  while(!kmsg){
    kmsg = me->messages.get(sender, msg->flags);
    if(!kmsg)
      wait(WFLAG_RECV);
  }
  if(kmsg == -1) return RES_FAULT;

  /*  подготовка сообщения  */
  kmsg.init(flags, msg);

  /* передача сообщения в пространство пользователя */
  messages.import(kmsg, msg);
  
  return RES_SUCCESS;
}

#warning Добавить lock страниц буфера при отправке (во избежание освобождения этих станиц другим потоком процесса)

#error ЗАКОНЧИТЬ ЭТУ ЧАСТЬ КОДА
res_t send(message *msg)
{
  if(check_message(msg, MSG_CHK_SENDBUF | MSG_CHK_RECVBUF | MSG_CHK_FLAGS))
    return RES_FAULT;

  Thread *thread; /* процесс-получатель */

  system->mt.disable();
  thread = THREAD(message->tid);

  //printk("send [%s]->[%s] \n", system->procman->current_thread->process->name, thread->process->name);
  
  if (!thread){
    message->send_size = 0;
    system->mt.enable();
    return RES_FAULT;
  }

  //  system->mt.enable(); /* #1 */

  if(thread->messages.unread.count.value() >= MAX_MSG_COUNT){
    message->send_size = 0;
    system->mt.enable();
    return RES_FAULT2;
  }

  Thread *my_thread = system->procman->current_thread;
  /* простое предупреждение взаимоблокировки */
  my_thread->send_to = thread->tid;
  if(thread->send_to == my_thread->tid){
    my_thread->send_to = 0;
    message->send_size = 0;
    system->mt.enable();
    return RES_FAULT3;
  }
  
  kmessage *send_message = new kmessage;
  send_message->flags = message->flags;
  send_message->size = message->send_size;

  if(send_message->size) {
    if(!(message->flags & (MSG_MEM_SEND | MSG_MEM_SHARE))) {
      /* скопируем сообщение в память ядра */
      send_message->buffer = new char[send_message->size];
      memcpy(send_message->buffer, message->send_buf, send_message->size);
    } else
      send_message->buffer = (void *) message->send_buf;
  }

  send_message->arg[0] = message->arg[0];
  send_message->arg[1] = message->arg[1];
  send_message->arg[2] = message->arg[2];
  send_message->arg[3] = message->arg[3];
  
  send_message->reply_size = message->recv_size;
  send_message->thread = my_thread;

  //system->mt.disable();
  thread->messages.unread.list.add_tail(send_message);       /* добавим сообщение процессу-получателю */
  thread->messages.unread.count.inc();
  //thread->flags &= ~FLAG_TSK_RECV;	         /* сбросим флаг ожидания получения сообщения (если он там есть) */
  thread->start(WFLAG_RECV);
  
  //send_message->thread->flags |= FLAG_TSK_SEND;	 /* ожидаем ответа */
  send_message->thread->wait(WFLAG_SEND);
  
  system->mt.enable();
  sched_yield();                                 /*  ожидаем ответа  */

  if(send_message->reply_size) { /* скопируем полученный ответ в память процесса */
    memcpy(message->recv_buf, send_message->buffer, send_message->reply_size);
    delete (u32_t *) send_message->buffer;
  }

  message->send_size = send_message->size;	  /* сколько байт дошло до получателя */
  message->recv_size = send_message->reply_size;  /* сколько байт ответа пришло */
  message->tid = send_message->thread->tid;       /* ответ на сообщение мог прийти не от изначального получателя,
						     а от другого процесса (при использовании получателем forward()) */
  message->pid = send_message->thread->process->pid;

  message->arg[0] = send_message->arg[0];
  message->arg[1] = send_message->arg[1];
  message->arg[2] = send_message->arg[2];
  message->arg[3] = send_message->arg[3];
  
  delete send_message;
  my_thread->send_to = 0;

  return RES_SUCCESS;
}

res_t reply(message *message)
{
  if(check_message(message, MSG_CHK_SENDBUF))
    return RES_FAULT;

  kmessage *send_message = 0;
  List<kmessage *> *entry;
  List<kmessage *> *messages = &system->procman->current_thread->messages.read.list;

  /* Ищем сообщение в списке полученных (чтобы ответ дошел, пользовательское приложение не должно менять поле tid) */
  system->mt.disable();
  list_for_each (entry, messages) {
    send_message = entry->item;
    if(send_message->thread == THREAD(message->tid))
      break;
  }
  system->mt.enable();

  if(!send_message || (send_message->thread->tid != message->tid)){
    message->send_size = 0;
    return RES_FAULT;
  }

  if(send_message->thread->flags & (FLAG_TSK_TERM | FLAG_TSK_EXIT_THREAD)) {
    /* поток-отправитель ожидает завершения */
    system->mt.disable();
    delete entry; /* удалим запись о сообщении из списка полученных сообщений */
    if(!(send_message->flags & MSG_ASYNC))
      send_message->thread->start(WFLAG_SEND); /* сбросим у отправителя флаг TSK_SEND */
      //send_message->thread->flags &= ~FLAG_TSK_SEND; /* сбросим у отправителя флаг TSK_SEND */
    system->mt.enable();
    return RES_SUCCESS;
  }
  //printk("reply [%s]->[0x%X]\n", system->procman->current_thread->process->name, send_message->thread /*->process->name*/);
  Thread *thread = send_message->thread;
  send_message->thread = system->procman->current_thread;

  if (send_message->reply_size < message->send_size)
    message->send_size = send_message->reply_size;
  else
    send_message->reply_size = message->send_size;

  if (send_message->reply_size) {
    send_message->buffer = new char[send_message->reply_size];
    memcpy(send_message->buffer, message->send_buf, send_message->reply_size);
  }

  send_message->arg[0] = message->arg[0];
  send_message->arg[1] = message->arg[1];
  send_message->arg[2] = message->arg[2];
  send_message->arg[3] = message->arg[3];

  system->mt.disable();
  delete entry; /* удалим запись о сообщении из списка полученных сообщений */
  //if(TID(thread) > 0x1000)
  thread->start(WFLAG_SEND); /* сбросим у отправителя флаг TSK_SEND */
  //thread->flags &= ~FLAG_TSK_SEND; /* сбросим у отправителя флаг TSK_SEND */
  system->mt.enable();

  return RES_SUCCESS;
}

/*
  переадресовать сообщение другому получателю
  отправитель сообщения не меняется
  должно вызываться только привилегированным потоком
*/
res_t forward(message *message, tid_t to)
{
  Thread *thread;
  system->mt.disable();
  thread = THREAD(to);

  if (!thread) {
    message->send_size = 0;
    system->mt.enable();
    return RES_FAULT;
  }

  if (thread->messages.unread.count.value() >= MAX_MSG_COUNT) {
    message->send_size = 0;
    system->mt.enable();
    return RES_FAULT2;
  }

  Thread *thread_sender = THREAD(message->tid);
  //printk("forward [%s]->[%s] \n", thread_sender->process->name , thread->process->name);

  /* простое предупреждение взаимоблокировки */
  thread_sender->send_to = thread->tid;
  if (thread->send_to == thread_sender->tid) {
    thread_sender->send_to = 0;
    message->send_size = 0;
    system->mt.enable();
    return RES_FAULT3;
  }

  kmessage *send_message = 0;
  List<kmessage *> *entry;
  List<kmessage *> *messages = &system->procman->current_thread->messages.read.list;

  /* Ищем сообщение в списке полученных */
  //system->mt.disable();
  list_for_each (entry, messages) {
    send_message = entry->item;
    if(send_message->thread == thread_sender /*THREAD(message->tid)*/)
      break;
  }
  //system->mt.enable();

  if(!send_message || (send_message->thread != thread_sender /*THREAD(message->tid)*/)){
    message->send_size = 0;
    system->mt.enable();
    return RES_FAULT;
  }

  //printk("forward [%s]->[%s] \n", send_message->thread->process->name , thread->process->name);
  
  send_message->size = message->send_size;
  if(send_message->size){
    send_message->buffer = new char[send_message->size];
    memcpy(send_message->buffer, message->send_buf, send_message->size);
    //printk("{%s}", message->send_buf);
  }

  send_message->thread = thread_sender;

  //system->mt.disable();
  entry->move_tail(&thread->messages.unread.list);
  thread->messages.unread.count.inc();
  system->procman->current_thread->messages.read.count.dec();
  thread->start(WFLAG_RECV);	         /* сбросим флаг ожидания получения сообщения (если он там есть) */
  //thread->flags &= ~FLAG_TSK_RECV;	         /* сбросим флаг ожидания получения сообщения (если он там есть) */
  system->mt.enable();

  return RES_SUCCESS;
}
