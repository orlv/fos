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

static void wait_message()
{
  system->procman->current_thread->wait(WFLAG_RECV);
  system->mt.enable();
  sched_yield();
  system->mt.disable();
}

static List<kmessage *> *get_any_message()
{
  system->mt.disable();
  if (!system->procman->current_thread->messages.unread.count.value())
    wait_message();

  return system->procman->current_thread->messages.unread.list.next;
}

static List<kmessage *> *get_message_from(Thread *from)
{
  system->mt.disable();
  List<kmessage *> *messages = &system->procman->current_thread->messages.unread.list;
  List<kmessage *> *entry;

  while(1) {
    list_for_each (entry, messages) {
      if(entry->item->thread == from)
	return entry;
    }

    if (from && system->procman->current_thread->messages.unread.count.value() > MAX_MSG_COUNT)
      return 0;
    
    wait_message();
  }
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

static kmessage * get_message(tid_t from, u32_t flags)
{
  List<kmessage *> *entry;

  if((flags & MSG_ASYNC))
    entry = get_message_from(0);
  else if(from)
    entry = get_message_from(THREAD(from));
  else
    entry = get_any_message();
  
  if(!entry) {
    system->mt.enable();
    return 0;
  }

  kmessage *message = entry->item;

#if 0 /* вариант проверки флагов с удалением неподходящих сообщений */
  if((flags & MSG_MEM_TAKE)) {
    if((message->flags & MSG_MEM_SEND)) break;
    else {
      delete entry;
      kill_message(message);
      continue;
    }
  } else if(flags & MSG_MEM_SHARE) {
    if((message->flags & MSG_MEM_SHARE)) break;
    else {
      delete entry;
      kill_message(message);
      continue;
    }
  } else if(message->flags & (MSG_MEM_SEND | MSG_MEM_SHARE)) {
    delete entry;
    kill_message(message);
  }
#endif
  
  /* сверяем флаги */
  if(flags & MSG_MEM_TAKE) {
    if(!(message->flags & MSG_MEM_SEND)){
      //printk("trunk1\n");
      message->size = 0; /* при несовпадении флагов не передаем буфер, передаем только аргументы */
    }
  } else if(flags & MSG_MEM_SHARE) {
    if(!(message->flags & MSG_MEM_SHARE)){
      //printk("trunk2\n");
      message->size = 0;
    }
  } else if((message->flags & (MSG_MEM_SEND | MSG_MEM_SHARE))) {
    //printk("trunk3\n");
    message->size = 0;
  }
  //break;
  //}
  Thread *current_thread = system->procman->current_thread;  
  //printk("[0x%X]", message->thread);
  if (!(message->flags & MSG_ASYNC)){ /* перемещаем сообщение в очередь полученных сообщений */
    entry->move_tail(&current_thread->messages.read.list);
    current_thread->messages.read.count.inc();
  } else
    delete entry; /* убираем сообщение из очереди новых сообщений */

  current_thread->messages.unread.count.dec();
  system->mt.enable();
  
  return message;
}

res_t receive(message *message)
{
  if(check_message(message, MSG_CHK_RECVBUF))
    return RES_FAULT;
  
  //printk("receive [%s]\n", system->procman->current_thread->process->name);
  kmessage *received_message = get_message(message->tid, message->flags);
  if(!received_message) return RES_FAULT;  

  size_t rcv_size = received_message->size;
  message->send_size = received_message->reply_size;

  /*  if((message->flags & (MSG_MEM_SEND | MSG_MEM_SHARE))){
    printk("SHM! rs=0x%X\n", rcv_size);
    }*/
  
  if (rcv_size > message->recv_size) /* решаем, сколько байт сообщения копировать */
    rcv_size = message->recv_size;
  else
    message->recv_size = rcv_size;
  
  if (rcv_size) {
    if(!(received_message->flags & (MSG_MEM_SEND | MSG_MEM_SHARE))) {
      /* копируем сообщение из ядра в память получателя */
      memcpy(message->recv_buf, received_message->buffer, rcv_size);
      delete (u32_t *) received_message->buffer; /* переданные данные больше не нужны в ядре, освобождаем память */
    } else {
      //printk("shared message!\n");
      rcv_size &= ~0xfff;
      if(rcv_size) {
	/* монтируем буфер в свободное место адр. пр-ва получателя */
	//printk("kernel: buf=[0x%X] rcv_size=[0x%X]", received_message->buffer, rcv_size);
	message->recv_buf = system->procman->current_thread->process->memory->mmap(0, rcv_size, 0, OFFSET(received_message->buffer), received_message->thread->process->memory);
	
	if(message->flags & MSG_MEM_SEND) /* демонтируем буфер из памяти отправителя */
	  received_message->thread->process->memory->munmap(OFFSET(received_message->buffer), received_message->size);
      }
    }
  }

  message->arg[0] = received_message->arg[0];
  message->arg[1] = received_message->arg[1];
  message->arg[2] = received_message->arg[2];
  message->arg[3] = received_message->arg[3];
  
  if((received_message->flags & MSG_ASYNC)) { 
    message->tid = 0;
    message->pid = 0;
    message->flags &= MSG_ASYNC;
    delete received_message; /* асинхронные сообщения не требуют ответа, сразу удаляем из ядра */
  } else {
    message->tid = received_message->thread->tid; /* укажем отправителя сообщения */
    message->pid = received_message->thread->process->pid;
  }

  return RES_SUCCESS;
}

#warning Добавить lock страниц буфера при отправке (во избежание освобождения этих станиц другим потоком процесса)

res_t send(message *message)
{
  if(check_message(message, MSG_CHK_SENDBUF | MSG_CHK_RECVBUF | MSG_CHK_FLAGS))
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
