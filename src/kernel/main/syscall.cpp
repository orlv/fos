/*
 * kernel/main/syscall.cpp
 * Copyright (C) 2005-2007 Oleg Fedorov
 */

#include <system.h>
#include <stdio.h>
#include <string.h>
#include <hal.h>

#define SEND              1
#define RECEIVE           2
#define REPLY             3
#define MASK_INTERRUPT    4
#define UNMASK_INTERRUPT  5
#define SCHED_YIELD       6
#define UPTIME            7

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
      "push %ecx \n"	      /* сохраним arg2 */			\
      "push %ebx \n"	      /* сохраним arg1 */			\
      "mov 48(%esp), %eax \n" /* сохраним eip */			\
      "push %eax \n"							\
      "xor %eax, %eax \n"						\
      "mov 48(%esp), %ax \n"  /* сохраним cs */				\
      "push %eax \n"							\
      "call _" #func " \n"						\
      "mov %eax, __result \n"						\
      "add $16, %esp \n"						\
      "pop %es \n"							\
      "pop %ds \n"							\
      "popa \n"								\
      "mov __result, %eax \n"						\
      "iret \n"								\
      "__result: .long 0");						\
  asmlinkage void _ ## func(unsigned int cs, unsigned int address, u32_t cmd, u32_t arg)

void outchar(char ch, u32_t off)
{
  char *ptr = (char *) (0xb8000 + off);
  ptr[0] = ch;
  ptr[1] = 0xf0;
}

kmessage * get_message()
{
  hal->mt_disable();
  Thread *current_thread = hal->ProcMan->CurrentThread;
  if (!current_thread->new_messages_count.read()) {  /* если нет ни одного входящего сообщения -- отключаемся в ожидании */
    /*  ожидание сообщения (отдаем управление планировщику, будем разблокированы по приходу сообщения) */
    hal->ProcMan->CurrentThread->flags |= FLAG_TSK_RECV;
    hal->mt_enable();
    sched_yield();
    hal->mt_disable();
  }
  
  /* пришло сообщение, обрабатываем.. */
  kmessage *message = current_thread->new_messages->next->item;
  if (!(message->flags & MESSAGE_ASYNC)){
    /* перемещаем сообщение в очередь полученных сообщений */
    current_thread->new_messages->next->move_tail(current_thread->received_messages);
    current_thread->received_messages_count.inc();
  } else {
    delete current_thread->new_messages->next; /* убираем сообщение из очереди новых сообщений */
  }
  current_thread->new_messages_count.dec();
  hal->mt_enable();

  return message;
}

void receive(message *message)
{
  size_t size;
  kmessage *received_message = get_message();

  message->send_size = received_message->reply_size;
  
  if (received_message->size > message->recv_size) /* решаем, сколько байт сообщения копировать */
    size = received_message->size = message->recv_size;
  else
    size = message->recv_size = received_message->size;
  
  if (size) /* копируем сообщение из ядра в память получателя */
    memcpy(message->recv_buf, received_message->buffer, size);

  message->tid = TID(received_message->thread); /* укажем отправителя сообщения */

  delete (u32_t *) received_message->buffer; /* переданные данные больше не нужны в ядре, освобождаем память */
  if((received_message->flags & MESSAGE_ASYNC)) /* асинхронные сообщения не требуют ответа, сразу удаляем из ядра */
    delete received_message;
}

res_t send(message *message)
{
  Thread *thread; /* поток-получатель */

  if(!message->tid){
    thread = THREAD(hal->tid_namer);
  } else {
    thread = THREAD(message->tid);
  }

  if (!hal->ProcMan->get_thread_by_tid(TID(thread)))
    return RES_FAULT;

  if(thread->new_messages_count.read() >= MAX_MSG_COUNT)
    return RES_FAULT2;

  Thread *current_thread = hal->ProcMan->CurrentThread;
  
  /* простое предупреждение взаимоблокировки */
  current_thread->send_to = TID(thread);
  if(thread->send_to == TID(current_thread)){
    current_thread->send_to = 0;
    return RES_FAULT;
  }

  /* скопируем сообщение в память ядра */
  kmessage *send_message = new kmessage;
  send_message->buffer = new char[message->send_size];
  send_message->size = message->send_size;
  memcpy(send_message->buffer, message->send_buf, send_message->size);

  send_message->reply_size = message->recv_size;
  send_message->thread = current_thread;
  
  hal->mt_disable();
  thread->new_messages->add_tail(send_message);       /* добавим сообщение процессу-получателю */
  thread->new_messages_count.inc();
  thread->flags &= ~FLAG_TSK_RECV;	         /* сбросим флаг ожидания получения сообщения (если он там есть) */
  send_message->thread->flags |= FLAG_TSK_SEND;	 /* ожидаем ответа */
  hal->mt_enable();
  sched_yield();                                 /*  ожидаем ответа  */

  /* скопируем полученный ответ в память процесса */
  memcpy(message->recv_buf, send_message->buffer, send_message->reply_size);
  message->send_size = send_message->size;	  /* сколько байт дошло до получателя */
  message->recv_size = send_message->reply_size;  /* сколько байт ответа пришло */
  message->tid = TID(send_message->thread);       /* ответ на сообщение мог придти не от изначального получателя,
						     а от другого процесса (при использовании получателем forward()) */
  
  delete (u32_t *) send_message->buffer; /* ответ на сообщение */
  delete send_message;
  current_thread->send_to = 0;
  return RES_SUCCESS;
}

void reply(message *message)
{
  size_t size;
  kmessage *send_message = 0;
  List<kmessage *> *entry;
  List<kmessage *> *messages = hal->ProcMan->CurrentThread->received_messages;

  /* Ищем сообщение в списке полученных (чтобы ответ дошел, пользовательское приложение не должно менять поле tid) */
  hal->mt_disable();
  list_for_each (entry, messages) {
    send_message = entry->item;
    if(send_message->thread == THREAD(message->tid))
      break;
  }
  hal->mt_enable();

  if(!send_message)
    return;

  Thread *thread = send_message->thread;
  send_message->thread = hal->ProcMan->CurrentThread;

  if (send_message->reply_size < message->send_size)
    size = message->send_size = send_message->reply_size;
  else
    size = send_message->reply_size = message->send_size;

  if (size) {
    send_message->buffer = new char[size];
    memcpy(send_message->buffer, message->send_buf, size);
  }

  hal->mt_disable();
  delete entry; /* удалим запись о сообщении из списка полученных сообщений */
  thread->flags &= ~FLAG_TSK_SEND; /* сбросим у отправителя флаг TSK_SEND */
  hal->mt_enable();
}

res_t forward(message *message, tid_t tid)
{
  Thread *thread; /* процесс-получатель */
  if(!(thread = hal->ProcMan->get_thread_by_tid(tid)))
    return RES_FAULT;


  kmessage *send_message = 0;
  List<kmessage *> *entry;
  List<kmessage *> *messages = hal->ProcMan->CurrentThread->received_messages;

  /* Ищем сообщение в списке полученных (чтобы ответ дошел, пользовательское приложение не должно менять поле pid) */
  //hal->cli();
  hal->mt_disable();
  list_for_each (entry, messages) {
    send_message = entry->item;
    if(send_message->thread == THREAD(message->tid))
      break;
  }
  //hal->sti();
  hal->mt_enable();
  
  if(!send_message)
    return RES_FAULT;

  //hal->cli();
  hal->mt_disable();
  thread->new_messages->add_tail(send_message);
  thread->flags &= ~FLAG_TSK_RECV;	/* сбросим флаг ожидания получения сообщения (если он там есть) */
  delete entry; /* удалим ссылку на сообщение из своей очереди */
  //hal->sti();
  hal->mt_enable();
  
  return RES_SUCCESS;
}

u32_t uptime();

SYSCALL_HANDLER(sys_call)
{
  //printk("\nSyscall #%d, Process %d ", cmd,  hal->ProcMan->CurrentThread->pid);
  switch (cmd) {

  case RECEIVE:
    receive((message *)arg);
    break;

  /*
    -----------------------------------------------------------------------------
    SEND: сообщение копируется в буфер, управление передаётся планировщику
    когда адресат, получив сообщение, делает вызов REPLY -- управление возвращается
    -----------------------------------------------------------------------------
  */
  case SEND:
    send((message *)arg);
    break;

  case REPLY:
    reply((message *)arg);
    break;

  case MASK_INTERRUPT:
    hal->pic->mask(arg);
    break;

  case UNMASK_INTERRUPT:
    hal->pic->unmask(arg);
    break;

  case SCHED_YIELD:
    sched_yield();
    break;

  case UPTIME:
    *(u32_t *)arg = uptime();
    break;
    
  default:
    break;
  }
}
