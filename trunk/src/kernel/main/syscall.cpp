/*
 * kernel/main/syscall.cpp
 * Copyright (C) 2005-2007 Oleg Fedorov
 */

#include <system.h>
#include <stdio.h>
#include <string.h>
#include <hal.h>
#include <mmu.h>

/* основные системные вызовы -- обмен сообщениями */
#define _FOS_SEND              1
#define _FOS_RECEIVE           2
#define _FOS_REPLY             3
#define _FOS_FORWARD           4

/*
  следующие функции целесообразнее разместить
  в системных вызовах -- очень существенно сказавается
  на производительности
*/
#define _FOS_MASK_INTERRUPT    5
#define _FOS_UNMASK_INTERRUPT  6
#define _FOS_SCHED_YIELD       7
#define _FOS_UPTIME            8
#define _FOS_ALARM             9
#define _FOS_MYTID            10 /* позволяет потоку узнать свой Thread ID */

/*
  
  3 функции обмена сообщениями:

  -- Процесс 1:
  
  send(msg);    // Отправить сообщение, разблокировать получателя (если он перед этим сделал вызов receive()), дождаться ответа

  -- Процесс 2:
  
  receive(msg); // Ждать сообщения (или получить уже пришедшее)
  reply(msg);   // Ответить отправителю, разблокировав его

  -------------------------------------------------------------------------
  -------------------------------------------------------------------------
  send(msg);

  message.send_buf;  // Указатель на буфер с сообщением в области приложения
  message.send_size; // Размер сообщения
  message.recv_buf;  // Буфер для ответного сообщения
  message.recv_size; // Максимальный размер ответа (размер буфера)

  message.pid;       // ID получателя

  -Если получателя не существует - сообщение теряется,
   возвращается управление
   
  -Сообщение копируется в память ядра (msg)
  -В список сообщений получателя добавляется указатель на msg
  -Процесс приостанавливается до обработки сообщения о отправки ответа получателем

  ------>>

  После возврата управления:
  
  msg.send_size; // Количество переданных байт
  msg.recv_buf;  // Буфер с ответом
  msg.recv_size; // Размер ответа

  Затем msg копируется в память приложения, буферы ядра удаляются

  -----------------------------------------------------------------------
  receive(msg);

  message.recv_buf;  // Буфер под сообщение
  message.recv_size; // Максимальный размер сообщения (размер буфера)

  message.pid;       // Отправитель сообщения
  
  -Если ни одного сообщения ещё не пришло, блокируемся в ожидании
  -При получении происходит разблокировка, копируем сообщение в приложение
  -ОТПРАВИТЕЛЬ ПРИ ЭТОМ ОСТАЁТСЯ ЗАБЛОКИРОВАН. Для его разблокировки необходимо сделать вызов reply()
  
  -----------------------------------------------------------------------
  reply(msg);

  message.send_buf;  // Указатель на буфер с ответом в области приложения
  message.send_size; // Размер ответа
  message.pid;       // Получатель ответа

  -Ответ копируется в память ядра
  -Отправитель разблокируется (и, после, сам копирует (в режиме ядра, естественно) ответ из ядра в приложение)
  -Возвращается управление, оба процесса продолжают работу
  
*/

struct memmap {
  u32_t ptr;
  u32_t size;
} __attribute__ ((packed));


#define SYSCALL_HANDLER(func) asmlinkage void func (u32_t cmd, u32_t arg); \
  asm(".globl " #func"\n"						\
      #func ": \n"							\
      "pusha \n"							\
      "push %ds \n"							\
      "push %es \n"							\
      "mov $0x10, %ax \n"     /* загрузим DS ядра */			\
      "mov %ax, %ds \n"							\
      "mov %ax, %es \n"							\
      "push %edx \n"	      /* сохраним arg3 */			\
      "push %ecx \n"	      /* сохраним arg2 */			\
      "push %ebx \n"	      /* сохраним arg1 */			\
      "mov 48(%esp), %eax \n" /* сохраним eip */			\
      "push %eax \n"							\
      "xor %eax, %eax \n"						\
      "mov 48(%esp), %ax \n"  /* сохраним cs */				\
      "push %eax \n"							\
      "call _" #func " \n"						\
      "mov %eax, __result \n"						\
      "add $20, %esp \n"						\
      "pop %es \n"							\
      "pop %ds \n"							\
      "popa \n"								\
      "mov __result, %eax \n"						\
      "iret \n"								\
      "__result: .long 0");						\
  asmlinkage u32_t _ ## func(unsigned int cs, unsigned int address, u32_t cmd, u32_t arg1, u32_t arg2)

void wait_message()
{
  hal->procman->current_thread->flags |= FLAG_TSK_RECV;
  hal->mt_enable();
  sched_yield();
  hal->mt_disable();
}

List<kmessage *> *get_message_any()
{
  hal->mt_disable();
  if (!hal->procman->current_thread->new_messages_count.read())
    wait_message();

  return hal->procman->current_thread->new_messages->next;
}

List<kmessage *> *get_message_from(tid_t from)
{
  hal->mt_disable();
  List<kmessage *> *messages = hal->procman->current_thread->new_messages;
  List<kmessage *> *entry;

  if (hal->procman->current_thread->new_messages_count.read()) { /* есть сообщения, обрабатываем.. */
    list_for_each (entry, messages) {
      if(entry->item->thread == THREAD(from))
	return entry;
    }
  }
  
  while(1) {
    if (!SYSTEM_TID(from) && hal->procman->current_thread->new_messages_count.read() > MAX_MSG_COUNT)
      return 0;
    
    wait_message();
    list_for_each (entry, messages) {
      if(entry->item->thread == THREAD(from))
	return entry;
    }
  }
}


#define MSG_CHK_SENDBUF  1
#define MSG_CHK_RECVBUF  2

static inline bool check_message(message *message, int flags)
{
  if(OFFSET(message) < hal->procman->current_thread->process->memory->mem_base)
    return 1;
  
  u32_t *pagedir = hal->procman->current_thread->process->memory->pagedir;
  u32_t count;

  count = (OFFSET(message)%PAGE_SIZE + sizeof(struct message) + PAGE_SIZE - 1)/PAGE_SIZE;

  if(!check_pages(PAGE(OFFSET(message)), pagedir, count))
    return 1;
    
  if((flags & MSG_CHK_RECVBUF) && message->recv_size) {
    count = (OFFSET(message->recv_buf)%PAGE_SIZE + message->recv_size + PAGE_SIZE - 1)/PAGE_SIZE;
    
    if(!check_pages(PAGE(OFFSET(message->recv_buf)), pagedir, count))
      return 1;
  }

  if((flags & MSG_CHK_SENDBUF) && message->send_size) {
    count = (OFFSET(message->send_buf)%PAGE_SIZE + message->send_size + PAGE_SIZE - 1)/PAGE_SIZE;
  
    if(!check_pages(PAGE(OFFSET(message->send_buf)), pagedir, count))
      return 1;
  }

  return 0;
}

kmessage * get_message(tid_t from)
{
  kmessage *message;
  List<kmessage *> *entry;
  Thread *current_thread = hal->procman->current_thread;

  if(from)
    entry = get_message_from(from);
  else
    entry = get_message_any();

  if(!entry) {
    hal->mt_enable();
    return 0;
  }

  message = entry->item;
  
  //printk("[0x%X]", message->thread);
  if (!(message->flags & MESSAGE_ASYNC)){ /* перемещаем сообщение в очередь полученных сообщений */
    entry->move_tail(current_thread->received_messages);
    current_thread->received_messages_count.inc();
  } else
    delete entry; /* убираем сообщение из очереди новых сообщений */

  current_thread->new_messages_count.dec();
  hal->mt_enable();

  return message;
}

res_t receive(message *message)
{
  if(check_message(message, MSG_CHK_RECVBUF))
    return RES_FAULT;

  //printk("receive [%s]\n", hal->procman->current_thread->process->name);
  kmessage *received_message = get_message(message->tid);
  if(!received_message)
    return RES_FAULT;
  
  message->send_size = received_message->reply_size;
  
  if (received_message->size > message->recv_size) /* решаем, сколько байт сообщения копировать */
    received_message->size = message->recv_size;
  else
    message->recv_size = received_message->size;
  
  if (received_message->size) { /* копируем сообщение из ядра в память получателя */
    memcpy(message->recv_buf, received_message->buffer, received_message->size);
    delete (u32_t *) received_message->buffer; /* переданные данные больше не нужны в ядре, освобождаем память */
  }
  
  message->tid = TID(received_message->thread); /* укажем отправителя сообщения */

  message->a0 = received_message->a0;
  message->a1 = received_message->a1;
  message->a2 = received_message->a2;
  message->a3 = received_message->a3;
  
  if((received_message->flags & MESSAGE_ASYNC)) /* асинхронные сообщения не требуют ответа, сразу удаляем из ядра */
    delete received_message;

  return RES_SUCCESS;
}

res_t send(message *message)
{
  if(check_message(message, MSG_CHK_SENDBUF|MSG_CHK_RECVBUF))
    return RES_FAULT;

  Thread *thread; /* процесс-получатель */

  hal->mt_disable();
  switch(message->tid){
  case SYSTID_NAMER:
    thread = THREAD(hal->tid_namer);
    break;

  case SYSTID_PROCMAN:
    thread = THREAD(hal->tid_procman);
    break;

  case 0:
    message->send_size = 0;
    return RES_FAULT;

  default:
    thread = hal->procman->get_thread_by_tid(message->tid);
  }

  //printk("send [%s]->[%s] \n", hal->procman->current_thread->process->name, thread->process->name);
  
  if (!thread){
    message->send_size = 0;
    hal->mt_enable();
    return RES_FAULT;
  }

  //  thread->enter_exit_lock();
  //  hal->mt_enable(); /* #1 */

  if(thread->new_messages_count.read() >= MAX_MSG_COUNT){
    message->send_size = 0;
    hal->mt_enable();
    return RES_FAULT2;
  }

  Thread *thread_sender = hal->procman->current_thread;
 
  /* простое предупреждение взаимоблокировки */
  thread_sender->send_to = TID(thread);
  if(thread->send_to == TID(thread_sender)){
    thread_sender->send_to = 0;
    message->send_size = 0;
    hal->mt_enable();
    return RES_FAULT3;
  }

  /* скопируем сообщение в память ядра */
  kmessage *send_message = new kmessage;
  send_message->size = message->send_size;
  if(send_message->size){
    send_message->buffer = new char[send_message->size];
    memcpy(send_message->buffer, message->send_buf, send_message->size);
  }

  send_message->a0 = message->a0;
  send_message->a1 = message->a1;
  send_message->a2 = message->a2;
  send_message->a3 = message->a3;
  
  send_message->reply_size = message->recv_size;
  send_message->thread = thread_sender;

  //hal->mt_disable();
  thread->new_messages->add_tail(send_message);       /* добавим сообщение процессу-получателю */
  thread->new_messages_count.inc();
  thread->flags &= ~FLAG_TSK_RECV;	         /* сбросим флаг ожидания получения сообщения (если он там есть) */
  send_message->thread->flags |= FLAG_TSK_SEND;	 /* ожидаем ответа */
  hal->mt_enable();
  sched_yield();                                 /*  ожидаем ответа  */

  if(send_message->reply_size) { /* скопируем полученный ответ в память процесса */
    memcpy(message->recv_buf, send_message->buffer, send_message->reply_size);
    delete (u32_t *) send_message->buffer;
  }

  message->send_size = send_message->size;	  /* сколько байт дошло до получателя */
  message->recv_size = send_message->reply_size;  /* сколько байт ответа пришло */
  message->tid = TID(send_message->thread);       /* ответ на сообщение мог придти не от изначального получателя,
						     а от другого процесса (при использовании получателем forward()) */
  message->a0 = send_message->a0;
  message->a1 = send_message->a1;
  message->a2 = send_message->a2;
  message->a3 = send_message->a3;
  
  delete send_message;
  thread_sender->send_to = 0;
  return RES_SUCCESS;
}

res_t reply(message *message)
{
  if(check_message(message, MSG_CHK_SENDBUF))
    return RES_FAULT;

  kmessage *send_message = 0;
  List<kmessage *> *entry;
  List<kmessage *> *messages = hal->procman->current_thread->received_messages;

  /* Ищем сообщение в списке полученных (чтобы ответ дошел, пользовательское приложение не должно менять поле tid) */
  hal->mt_disable();
  list_for_each (entry, messages) {
    send_message = entry->item;
    if(send_message->thread == THREAD(message->tid))
      break;
  }
  hal->mt_enable();

  if(!send_message){
    message->send_size = 0;
    return RES_FAULT;
  }

  if(send_message->thread->flags & (FLAG_TSK_TERM | FLAG_TSK_EXIT_THREAD)) {
    /* поток-отправитель ожидает завершения */
    hal->mt_disable();
    send_message->reply_size = 0;
    delete entry; /* удалим запись о сообщении из списка полученных сообщений */
    if(TID(send_message->thread) > 0x1000)
      send_message->thread->flags &= ~FLAG_TSK_SEND; /* сбросим у отправителя флаг TSK_SEND */
    hal->mt_enable();
    return RES_SUCCESS;
  }

  //printk("reply [%s]->[0x%X]\n", hal->procman->current_thread->process->name, send_message->thread /*->process->name*/);
  
  Thread *thread = send_message->thread;
  send_message->thread = hal->procman->current_thread;

  if (send_message->reply_size < message->send_size)
    message->send_size = send_message->reply_size;
  else
    send_message->reply_size = message->send_size;

  if (send_message->reply_size) {
    send_message->buffer = new char[send_message->reply_size];
    memcpy(send_message->buffer, message->send_buf, send_message->reply_size);
  }

  send_message->a0 = message->a0;
  send_message->a1 = message->a1;
  send_message->a2 = message->a2;
  send_message->a3 = message->a3;

  hal->mt_disable();
  delete entry; /* удалим запись о сообщении из списка полученных сообщений */
  if(TID(thread) > 0x1000)
    thread->flags &= ~FLAG_TSK_SEND; /* сбросим у отправителя флаг TSK_SEND */
  hal->mt_enable();
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
  hal->mt_disable();
  switch(to){
  case SYSTID_NAMER:
    thread = THREAD(hal->tid_namer);
    break;

  case SYSTID_PROCMAN:
    thread = THREAD(hal->tid_procman);
    break;

  case 0:
    message->send_size = 0;
    hal->mt_enable();
    return RES_FAULT;

  default:
    thread = hal->procman->get_thread_by_tid(to);
  }

  if (!thread){
    message->send_size = 0;
    hal->mt_enable();
    return RES_FAULT;
  }

  if(thread->new_messages_count.read() >= MAX_MSG_COUNT){
    message->send_size = 0;
    hal->mt_enable();
    return RES_FAULT2;
  }

  Thread *thread_sender = THREAD(message->tid);
  //printk("forward [%s]->[%s] \n", thread_sender->process->name , thread->process->name);

  /* простое предупреждение взаимоблокировки */
  thread_sender->send_to = TID(thread);
  if(thread->send_to == TID(thread_sender)){
    thread_sender->send_to = 0;
    message->send_size = 0;
    hal->mt_enable();
    return RES_FAULT3;
  }

  kmessage *send_message = 0;
  List<kmessage *> *entry;
  List<kmessage *> *messages = hal->procman->current_thread->received_messages;

  /* Ищем сообщение в списке полученных */
  //hal->mt_disable();
  list_for_each (entry, messages) {
    send_message = entry->item;
    if(send_message->thread == THREAD(message->tid))
      break;
  }
  //hal->mt_enable();

  if(!send_message){
    message->send_size = 0;
    hal->mt_enable();
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

  //hal->mt_disable();
  entry->move_tail(thread->new_messages);
  thread->new_messages_count.inc();
  hal->procman->current_thread->received_messages_count.dec();
  thread->flags &= ~FLAG_TSK_RECV;	         /* сбросим флаг ожидания получения сообщения (если он там есть) */
  hal->mt_enable();

  return RES_SUCCESS;
}

u32_t uptime();

void syscall_enter()
{
  hal->procman->current_thread->flags |= FLAG_TSK_SYSCALL;
}

void syscall_exit()
{
  hal->procman->current_thread->flags &= ~FLAG_TSK_SYSCALL;
  if((hal->procman->current_thread->flags & FLAG_TSK_TERM) || (hal->procman->current_thread->flags & FLAG_TSK_EXIT_THREAD)) {
    hal->procman->current_thread->flags &= ~FLAG_TSK_READY;
    sched_yield();
  }
}

SYSCALL_HANDLER(sys_call)
{
  //printk("Syscall #%d (%s) arg1=0x%X, arg2=0x%X \n", cmd,  hal->procman->current_thread->process->name, arg1, arg2);
  syscall_enter(); /* установим флаг нахождения в ядре */
  u32_t result = 0;
  u32_t _uptime;
  switch (cmd) {

  case _FOS_RECEIVE:
    result = receive((message *)arg1);
    break;

  /*
    -----------------------------------------------------------------------------
    _FOS_SEND: сообщение копируется в буфер, управление передаётся планировщику
    когда адресат, получив сообщение, делает вызов REPLY -- управление возвращается
    -----------------------------------------------------------------------------
  */
  case _FOS_SEND:
    result = send((message *)arg1);
    break;

  case _FOS_REPLY:
    result = reply((message *)arg1);
    break;

  case _FOS_FORWARD:
    result = forward((message *)arg1, arg2);
    break;
    
  case _FOS_MASK_INTERRUPT:
    hal->pic->mask(arg1);
    break;

  case _FOS_UNMASK_INTERRUPT:
    hal->pic->unmask(arg1);
    break;

  case _FOS_SCHED_YIELD:
    sched_yield();
    result = 0;
    break;

  case _FOS_UPTIME:
    result = uptime();
    break;

  case _FOS_ALARM:
    _uptime = uptime();
    if(hal->procman->current_thread->get_alarm() > _uptime)
      result = hal->procman->current_thread->get_alarm() - _uptime;
    else
      result = 0;
    
    if(arg1)  /* текущее время + arg1 */
      hal->procman->current_thread->set_alarm(_uptime + arg1);
    else
      hal->procman->current_thread->set_alarm(arg2);

    break;

  case _FOS_MYTID:
    result = TID(hal->procman->current_thread);
    break;
    
  default:
    result = RES_FAULT;
    break;
  }

  syscall_exit(); /* проверим, не убили ли нас ;) */
  return result;
}