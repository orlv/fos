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


#define SYSCALL_HANDLER(func) asmlinkage void func (u32_t arg1, u32_t arg2); \
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
      "add $16, %esp \n"						\
      "pop %es \n"							\
      "pop %ds \n"							\
      "popa \n"								\
      "iret");								\
  asmlinkage void _ ## func(unsigned int cs, unsigned int address, u32_t cmd, u32_t arg)

void receive(message *message)
{
  kmessage *msg;
  size_t size;

  /* если нет ни одного входящего сообщения, отключаемся в ожидании */
  if (hal->ProcMan->CurrentThread->new_msg->empty()) {
    hal->ProcMan->CurrentThread->flags |= FLAG_TSK_RECV;
    sched_yield();
  }

  /* SEND --> */
#warning ЗАМЕНИТЬ cli() на мьютекс!
  hal->cli();
  msg = hal->ProcMan->CurrentThread->new_msg->next->item;
  delete hal->ProcMan->CurrentThread->new_msg->next; /* убираем сообщение из очереди новых сообщений */
  hal->ProcMan->CurrentThread->recvd_msg->add_tail(msg); /* помещаем сообщение в очередь обрабатываемых сообщений */

  /* решаем, сколько байт сообщения копировать */
  if (msg->send_size > message->recv_size)
    size = message->recv_size;
  else
    size = msg->send_size;

  if (size) {     /* копируем сообщение из ядра в процесс */
    memcpy((u32_t *) message->recv_buf, (u32_t *) msg->send_buf, size);
  }

  message->tid = TID(msg->thread); /* не забываем указать отправителя сообщения */
  
  msg->send_size = message->recv_size = size; /* отметим, сколько байт было передано */

  delete (u32_t *)msg->send_buf; /* полученные данные больше не нужны в ядре, освобождаем память */
  hal->sti();
}

res_t send(message *message)
{
  //printk("msg: 0x%X->0x%X\n", message, message->tid);
  kmessage *msg;
  Thread *thread; /* процесс-получатель */
  if(!message->tid){
    thread = THREAD(hal->tid_namer);
  } else {
    thread = THREAD(message->tid);
  }
  if (!hal->ProcMan->get_thread_by_tid(TID(thread)))
    return RES_FAULT;

  /* простое предупреждение взаимоблокировки */
  hal->ProcMan->CurrentThread->send_to = TID(thread);
  if(thread->send_to == TID(hal->ProcMan->CurrentThread)){
    hal->ProcMan->CurrentThread->send_to = 0;
    return RES_FAULT;
  }

  /* скопируем сообщение в память ядра */
  msg = new kmessage;
  msg->send_buf = new char[message->send_size];
  msg->send_size = message->send_size;
  memcpy((u32_t *) msg->send_buf, (u32_t *) message->send_buf,
	 msg->send_size);

  msg->recv_buf = 0;
  msg->recv_size = message->recv_size;
  msg->thread = hal->ProcMan->CurrentThread;

#warning ЗАМЕНИТЬ cli() на мьютекс!
  hal->cli();

  thread->new_msg->add_tail(msg);       /* добавим сообщение процессу-получателю */
  thread->flags &= ~FLAG_TSK_RECV;	/* сбросим флаг ожидания получения сообщения (если он там есть) */
  msg->thread->flags |= FLAG_TSK_SEND;	/* ожидаем ответа */
  hal->sti();
  sched_yield();

  /* REPLY --> */
  /* скопируем полученные ответ в память процесса */
  memcpy((u32_t *) message->recv_buf, (u32_t *) msg->recv_buf,
	 msg->recv_size);
  message->send_size = msg->send_size;	/* сколько байт дошло до получателя */
  message->recv_size = msg->recv_size;	/* сколько байт ответа пришло */

  message->tid = TID(msg->thread); /* ответ на сообщение мог придти не от изначального получателя,
			      а от другого процесса (при использовании получателем forward()) */
  
  delete(u32_t *) msg->recv_buf; /* ответ на сообщение */
  delete msg;
  hal->ProcMan->CurrentThread->send_to = 0;
  return RES_SUCCESS;
}

res_t send_async(message *message)
{
  kmessage *msg;
  Thread *thread; /* поток-получатель */
  if (!(thread = hal->ProcMan->get_thread_by_tid(message->tid))){
    return RES_FAULT;
  }

  /* скопируем сообщение в память ядра */
  msg = new kmessage;
  msg->send_buf = new char[message->send_size];
  msg->send_size = message->send_size;
  memcpy((u32_t *) msg->send_buf, (u32_t *) message->send_buf,
	 msg->send_size);

  msg->flags = MESSAGE_ASYNC; /* не требует ответа - ответ игнорируется */

  msg->thread = hal->ProcMan->CurrentThread;

#warning ЗАМЕНИТЬ cli() на мьютекс!
  hal->cli();
  thread->new_msg->add_tail(msg);
  thread->flags &= ~FLAG_TSK_RECV;	/* сбросим флаг ожидания получения сообщения (если он там есть) */
  hal->sti();

  return RES_SUCCESS;
}


void reply(message *message)
{
  size_t size;
  kmessage *msg = 0;
  List<kmessage *> *entry;
  Thread *thread;
  /* Ищем сообщение в списке полученных (чтобы ответ дошел, пользовательское приложение не должно менять поле tid) */
  list_for_each (entry, hal->ProcMan->CurrentThread->recvd_msg) {
    msg = entry->item;
    if(msg->thread == THREAD(message->tid))
      break;
  }

  if(!msg || !msg->thread) /* процесс 0 не отправляет никому сообщения,
		 так что будем считать - это ответ на
		 несуществующее сообщение */
    return;
  
  /* на асинхронные сообщения не требуется ответа */
  if (msg->flags & MESSAGE_ASYNC) {
    delete msg;
    delete entry; /* удалим запись о сообщении из списка полученных сообщений */
  } else {
    thread = msg->thread;
    msg->thread = hal->ProcMan->CurrentThread;
  
    if (msg->recv_size < message->send_size)
      size = msg->recv_size;
    else
      size = message->send_size;

    msg->recv_size = message->send_size = size;
    if (size) {
      msg->recv_buf = new char[size];
      memcpy((u32_t *) msg->recv_buf, (u32_t *) message->send_buf, size);
    }

#warning ЗАМЕНИТЬ cli() на мьютекс!
    hal->cli();
    delete entry; /* удалим запись о сообщении из списка полученных сообщений */
    thread->flags &= ~FLAG_TSK_SEND; /* сбросим флаг SEND */
    hal->sti();
  }
}

res_t forward(message *message, tid_t tid)
{
  Thread *thread; /* процесс-получатель */
  if (!(thread = hal->ProcMan->get_thread_by_tid(tid))){
    return RES_FAULT;
  }

  kmessage *msg = 0;
  List<kmessage *> *entry;
  /* Ищем сообщение в списке полученных (чтобы ответ дошел, пользовательское приложение не должно менять поле pid) */
  list_for_each (entry, hal->ProcMan->CurrentThread->recvd_msg) {
    msg = entry->item;
    if(msg->thread == THREAD(message->tid))
      break;
  }

  if(!msg || !msg->thread) /* процесс 0 не отправляет никому сообщения,
		 так что будем считать - это ответ на
		 несуществующее сообщение */
    return RES_FAULT;

  //msg = (struct kmessage *)hal->ProcMan->CurrentThread->new_msg->next->data;

#warning ЗАМЕНИТЬ cli() на мьютекс!
  hal->cli();
  thread->new_msg->add_tail(msg);
  thread->flags &= ~FLAG_TSK_RECV;	/* сбросим флаг ожидания получения сообщения (если он там есть) */
  hal->sti();

  delete entry; /* удалим ссылку на сообщение из своей очереди */
  return RES_SUCCESS;
}

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
    
  default:
    break;
  }
}
