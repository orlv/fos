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

List<kmessage *> *msg_list::get(Thread *sender, u32_t flags)
{
  List<kmessage *> *entry;
  /* пройдём по списку в поисках нужного сообщения */
  list_for_each (entry, (&list)) {
    if(entry->item->thread == sender)
      return entry;
  }
  /* сообщение не найдено, и не придёт */
  if(sender && count.value() > MAX_MSG_COUNT)
    return (List<kmessage *> *)-1;
      
  /* сообщение не найдено, но можно повторить попытку позже */
  return 0;
}

/* находит сообщение в списке непрочтённых */
kmessage *Messenger::get(Thread *sender, u32_t flags)
{
  kmessage *msg;
  List<kmessage *> *entry;

  if((sender) || (flags & MSG_ASYNC)) {
    entry = unread.get(sender, flags);
  } else { /* любое сообщение */
    entry = (unread.count.value())?(unread.list.next):(0);
  }

  if(!entry || ((s32_t)entry == -1)) return (kmessage *)entry; /* возврат ошибки */

  msg = entry->item;
  if (!(msg->flags & MSG_ASYNC)){ /* перемещаем сообщение в очередь полученных сообщений */
    entry->move_tail(&read.list);
    read.count.inc();
  } else /* убираем сообщение из списков */
    delete entry; 

  unread.count.dec();
  return msg;
}

res_t Messenger::put_message(kmessage *message)
{
  system->preempt.disable();
  if(unread.count.value() >= MAX_MSG_COUNT){
    system->preempt.enable();
    return RES_FAULT2;
  }

  unread.list.add_tail(message);
  unread.count.inc();

  thread->start(TSTATE_WAIT_ON_RECV);

  system->preempt.enable();
  return RES_SUCCESS;
}

#define MSG_CHK_SENDBUF  1
#define MSG_CHK_RECVBUF  2
#define MSG_CHK_FLAGS    4

#if 0
static void kill_message(kmessage *message)
{
  message->reply_size = 0;
  message->size = 0;
  Thread *thread = message->thread;
  if(!(message->flags & MSG_ASYNC))
    thread->start(TSTATE_WAIT_ON_SEND);
  //thread->flags &= ~FLAG_TSK_SEND; /* сбросим у отправителя флаг TSK_SEND */
}
#endif

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

void kmessage::check_size(message *msg)
{
  /* проверим флаги */
  if(msg->flags & MSG_MEM_TAKE) {
    if(!(flags & MSG_MEM_SEND))
      size = 0; /* при несовпадении флагов не передаем буфер, передаем только аргументы */
  } else if(msg->flags & MSG_MEM_SHARE) {
    if(!(flags & MSG_MEM_SHARE))
      size = 0;
  } else if((flags & (MSG_MEM_SEND | MSG_MEM_SHARE)))
    size = 0;

  msg->send_size = reply_size;
  
  /* проверим соответствие размеров буферов
     решаем, сколько байт сообщения копировать */
  if(size > msg->recv_size)
    size = msg->recv_size;
  else
    msg->recv_size = size;
}

/* экспорт сообщения в пользовательское пространство памяти */
void Messenger::move_to_userspace(kmessage *kmsg, message *msg)
{
  if (kmsg->size) {
    size_t size = kmsg->size;
    if(!(kmsg->flags & (MSG_MEM_SEND | MSG_MEM_SHARE))) {
      memcpy(msg->recv_buf, kmsg->buffer, size); /* копируем сообщение из ядра в память получателя */
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

/* импорт сообщения в пространство памяти ядра */
kmessage *Messenger::import(message *msg, Thread *sender)
{
  kmessage *kmsg = new kmessage;
  kmsg->flags = msg->flags;
  kmsg->size  = msg->send_size;

  if(kmsg->size) {
    if(!(msg->flags & (MSG_MEM_SEND | MSG_MEM_SHARE))) {
      /* скопируем сообщение в память ядра */
      kmsg->buffer = new char[kmsg->size];
      memcpy(kmsg->buffer, msg->send_buf, kmsg->size);
    } else
      kmsg->buffer = (void *) msg->send_buf;
  }

  kmsg->arg[0] = msg->arg[0];
  kmsg->arg[1] = msg->arg[1];
  kmsg->arg[2] = msg->arg[2];
  kmsg->arg[3] = msg->arg[3];
  
  kmsg->reply_size = msg->recv_size;
  kmsg->thread = sender;

  unread.list.add_tail(kmsg);       /* добавим сообщение процессу-получателю */
  unread.count.inc();
  
  return kmsg;
}

res_t receive(message *msg)
{
  if(check_message(msg, MSG_CHK_RECVBUF))
    return RES_FAULT;

  kmessage *kmsg = 0;
  Thread *me = system->procman->current_thread;
  Thread *sender = (msg->tid)?(THREAD(msg->tid)):(0);

  system->preempt.disable();
    
  do {
    //me->wflag = 1;
    kmsg = me->messages.get(sender, msg->flags);
    if(!kmsg)
      me->wait(TSTATE_WAIT_ON_RECV);
  } while(!kmsg);
  if((s32_t)kmsg == -1) return RES_FAULT;

  /*  подготовка сообщения  */
  kmsg->check_size(msg);

  /* вывод сообщения в пространство пользователя */
  me->messages.move_to_userspace(kmsg, msg);
  
  return RES_SUCCESS;
}

#warning Добавить lock страниц буфера при отправке (во избежание освобождения этих станиц другим потоком процесса)

res_t send(message *msg)
{
  if(check_message(msg, MSG_CHK_SENDBUF | MSG_CHK_RECVBUF | MSG_CHK_FLAGS))
    return RES_FAULT;

  Thread *recipient; /* процесс-получатель */

  system->preempt.disable();
  recipient = THREAD(msg->tid);

  //printk("send [%s]->[%s] \n", system->procman->current_thread->process->name, recipient->process->name);

  /* 
   * Проверки возможности отправки сообщения:
   */

  /* получателя не существует */
  if (!recipient){
    msg->send_size = 0;
    return RES_FAULT;
  }

  /* очередь сообщений получателя переполнена */
  if(recipient->messages.unread.count.value() >= MAX_MSG_COUNT){
    msg->send_size = 0;
    return RES_FAULT2;
  }

  Thread *me = system->procman->current_thread;

  me->send_to = recipient->tid;
  /* взаимоблокировка */
  if(recipient->send_to == me->tid){
    me->send_to = 0;
    msg->send_size = 0;
    return RES_FAULT3;
  }

  /*
   * Отправка сообщения:
   */

  /* добавляем получателю сообщение  */
  kmessage *kmsg = recipient->messages.import(msg, me);

  preempt_disable();
  /* Остановка. Ждём ответа */
  me->wait(TSTATE_WAIT_ON_SEND);

  /* Разблокируем получателя */
  recipient->start(TSTATE_WAIT_ON_RECV);

  sched_yield();
  preempt_enable();
  
  //me->wstate = 1;
  //system->procman->stop(me);
  //system->preempt.enable();
  //sched_yield();

  /*
   * Обрабатываем ответ:
   */
  
  /* скопируем полученный ответ в память процесса */
  if(kmsg->reply_size) {
    memcpy(msg->recv_buf, kmsg->buffer, kmsg->reply_size);
    delete (u32_t *) kmsg->buffer;
  }

  msg->send_size = kmsg->size;	      /* размер дошедших данных (байт) */
  msg->recv_size = kmsg->reply_size;  /* размер ответа (байт) */
  msg->tid = kmsg->thread->tid;       /* ответ мог прийти не от изначального получателя,
					 а от другого процесса (при использовании переадресации получателем) */
  msg->pid = me->process->pid;

  msg->arg[0] = kmsg->arg[0];
  msg->arg[1] = kmsg->arg[1];
  msg->arg[2] = kmsg->arg[2];
  msg->arg[3] = kmsg->arg[3];
  
  delete kmsg;
  me->send_to = 0;

  return RES_SUCCESS;
}

res_t reply(message *msg)
{
  if(check_message(msg, MSG_CHK_SENDBUF))
    return RES_FAULT;

  Thread *me = system->procman->current_thread;
  kmessage *kmsg = 0;
  List<kmessage *> *entry;
  List<kmessage *> *messages = &me->messages.read.list;

  system->preempt.disable();
  Thread *sender = THREAD(msg->tid);

  if(!sender || (msg->flags & MSG_ASYNC)) {
    msg->send_size = 0;
    return RES_FAULT;
  }

  /* Ищем сообщение в списке полученных */
  list_for_each (entry, messages) {
    if(entry->item->thread == sender){
      kmsg = entry->item;
      break;
    }
  }

  if(!kmsg) {
    msg->send_size = 0;
    return RES_FAULT;
  }

  if(sender->flags & (FLAG_TSK_TERM | FLAG_TSK_EXIT_THREAD)) {
    /* поток-отправитель ожидает завершения */
    delete entry; /* удалим запись о сообщении из списка полученных сообщений */
    //system->procman->activate(kmsg->thread->me);
    kmsg->thread->start(TSTATE_WAIT_ON_SEND); /* сбросим у отправителя флаг SEND */
    return RES_SUCCESS;
  }
  //printk("reply [%s]->[0x%X]\n", system->procman->current_thread->process->name, send_message->thread /*->process->name*/);
  kmsg->thread = me;

  if(kmsg->reply_size < msg->send_size)
    msg->send_size = kmsg->reply_size;
  else
    kmsg->reply_size = msg->send_size;

  if(kmsg->reply_size) {
    kmsg->buffer = new char[kmsg->reply_size];
    memcpy(kmsg->buffer, msg->send_buf, kmsg->reply_size);
  }

  kmsg->arg[0] = msg->arg[0];
  kmsg->arg[1] = msg->arg[1];
  kmsg->arg[2] = msg->arg[2];
  kmsg->arg[3] = msg->arg[3];

  delete entry; /* удалим запись о сообщении из списка полученных сообщений */
  //sender->start(WFLAG_SEND); /* сбросим у отправителя флаг SEND */
  system->procman->activate(sender->me);

  return RES_SUCCESS;
}

/*
  переадресовать сообщение другому получателю
  отправитель сообщения не меняется
  должно вызываться только привилегированным потоком
*/
res_t forward(message *message, tid_t to)
{
  Thread *recipient;

  system->preempt.disable();
  recipient = THREAD(to);

  if (!recipient) {
    message->send_size = 0;
    return RES_FAULT;
  }

  if (recipient->messages.unread.count.value() >= MAX_MSG_COUNT) {
    message->send_size = 0;
    return RES_FAULT2;
  }

  Thread *sender = THREAD(message->tid);
  //printk("forward [%s]->[%s] \n", thread_sender->process->name , thread->process->name);

  /* простое предупреждение взаимоблокировки */
  sender->send_to = recipient->tid;
  if (recipient->send_to == sender->tid) {
    sender->send_to = 0;
    message->send_size = 0;
    return RES_FAULT3;
  }

  kmessage *kmsg = 0;
  List<kmessage *> *entry;
  List<kmessage *> *messages = &system->procman->current_thread->messages.read.list;

  /* Ищем сообщение в списке полученных */
  list_for_each (entry, messages) {
    if(entry->item->thread == sender) {
      kmsg = entry->item;
      break;
    }
  }

  if(!kmsg){
    message->send_size = 0;
    return RES_FAULT;
  }

  //printk("forward [%s]->[%s] \n", send_message->thread->process->name , thread->process->name);
  kmsg->size = message->send_size;
  if(kmsg->size){
    kmsg->buffer = new char[kmsg->size];
    memcpy(kmsg->buffer, message->send_buf, kmsg->size);
  }

  kmsg->thread = sender;

  entry->move_tail(&recipient->messages.unread.list);
  recipient->messages.unread.count.inc();
  system->procman->current_thread->messages.read.count.dec();
  recipient->start(TSTATE_WAIT_ON_RECV);	         /* сбросим флаг ожидания получения сообщения (если он там есть) */

  return RES_SUCCESS;
}
