/*
 * kernel/main/syscall.cpp
 * Copyright (C) 2005-2007 Oleg Fedorov
 */

#include <system.h>
#include <stdio.h>
#include <string.h>
#include <hal.h>

//void putpixel(u32_t offs, u16_t code);

#define SEND      1
#define RECEIVE   2
#define REPLY     3

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
  msg.recv_buf;  // Буфер с ответным сообщением
  msg.recv_size; // Размер ответа

  Затем msg копируется в память приложения, буферы ядра удаляются

  -----------------------------------------------------------------------
  receive(msg);

  message.recv_buf;  // Буфер под сообщение
  message.recv_size; // Максимальный размер сообщения (размер буфера)
  
  -Если ни одного сообщения ещё не пришло, блокируемся в ожидании
  -При получении происходит разблокировка, копируем сообщение в приложение
  -ОТПРАВИТЕЛЬ ПРИ ЭТОМ ОСТАЁТСЯ ЗАБЛОКИРОВАН. Для его разблокировки необходимо сделать вызов reply()
  
  -----------------------------------------------------------------------
  reply(msg);

  message.send_buf;  // Указатель на буфер с ответом в области приложения
  message.send_size; // Размер ответа

  -Ответ копируется в память ядра
  -Отправитель разблокируется (и, после, сам копирует ответ из ядра в приложение),
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

void receive(struct message *message)
{
  struct message *msg;
  size_t size;

  /* если нет ни одного входящего сообщения, отключаемся в ожидании */
  if (hal->ProcMan->CurrentProcess->msg->empty()) {
    hal->ProcMan->CurrentProcess->flags |= FLAG_TSK_RECV;
    pause();
  }

  /* SEND-->> */
#warning ЗАМЕНИТЬ cli() на мьютекс!
  hal->cli();
  msg = (struct message *)hal->ProcMan->CurrentProcess->msg->next->data;

  if (msg->send_size > message->recv_size)
    size = message->recv_size;
  else
    size = msg->send_size;

  if (size) {
    /* копируем сообщение из ядра в процесс */
    memcpy((u32_t *) message->recv_buf, (u32_t *) msg->send_buf, size);
  }

  message->pid = msg->pid;
  
  /* отметим, сколько байт было передано */
  msg->send_size = message->recv_size = size;

  delete(u32_t *) msg->send_buf;
  hal->sti();
}

res_t send(struct message *message)
{
  struct kmessage *msg;
  TProcess *p; /* процесс-получатель */
  if (!(p = hal->ProcMan->get_process_by_pid(message->pid))){
    return RES_FAULT;
  }

  /* простое предупреждение взаимоблокировки */
  hal->ProcMan->CurrentProcess->send_to = message->pid;
  if(p->send_to == hal->ProcMan->CurrentProcess->pid){
    return RES_FAULT;
  }
  
  /* скопируем сообщение в память ядра */
  msg = new(struct kmessage);
  msg->send_buf = new char[message->send_size];
  msg->send_size = message->send_size;
  memcpy((u32_t *) msg->send_buf, (u32_t *) message->send_buf,
	 msg->send_size);

  msg->recv_buf = 0;
  msg->recv_size = message->recv_size;
  msg->pid = hal->ProcMan->CurrentProcess->pid;
  msg->process = hal->ProcMan->CurrentProcess;

#warning ЗАМЕНИТЬ cli() на мьютекс!
  hal->cli();
  p->msg->add_tail(msg);

  p->flags &= ~FLAG_TSK_RECV;	/* сбросим флаг ожидания получения сообщения (если он там есть) */
  msg->process->flags |= FLAG_TSK_SEND;	/* ожидаем ответа */
  hal->sti();
  pause();

  /* REPLY-->> */
  memcpy((u32_t *) message->recv_buf, (u32_t *) msg->recv_buf,
	 msg->recv_size);
  message->send_size = msg->send_size;	/* сколько байт дошло до получателя */
  message->recv_size = msg->recv_size;	/* сколько байт ответа пришло */

  message->pid = msg->pid;
  
  delete(u32_t *) msg->recv_buf;
  delete msg;
  hal->ProcMan->CurrentProcess->send_to = 0;
  return RES_SUCCESS;
}

void reply(struct message *message)
{
  size_t size;
  struct kmessage *msg;
  msg = (struct kmessage *)hal->ProcMan->CurrentProcess->msg->next->data;

  msg->pid = hal->ProcMan->CurrentProcess->pid;
  
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
  msg->process->flags &= ~FLAG_TSK_SEND;	/* сбросим флаг SEND */
  delete hal->ProcMan->CurrentProcess->msg->next;
  hal->sti();
  //pause();
}

res_t forward(struct message *message, pid_t pid)
{
  TProcess *p; /* процесс-получатель */
  if (!(p = hal->ProcMan->get_process_by_pid(pid))){
    return RES_FAULT;
  }

  struct kmessage *msg;
  msg = (struct kmessage *)hal->ProcMan->CurrentProcess->msg->next->data;

#warning ЗАМЕНИТЬ cli() на мьютекс!
  hal->cli();
  p->msg->add_tail(msg);
  p->flags &= ~FLAG_TSK_RECV;	/* сбросим флаг ожидания получения сообщения (если он там есть) */
  hal->sti();

  delete hal->ProcMan->CurrentProcess->msg->next; /* удалим ссылку на сообщение из своей очереди */

  return RES_SUCCESS;
}

SYSCALL_HANDLER(sys_call)
{
  //printk("\nSyscall #%d, Process %d ", cmd,  hal->ProcMan->CurrentProcess->pid);
  switch (cmd) {

  case RECEIVE:
    receive((struct message *)arg);
    break;

  /*
    -----------------------------------------------------------------------------
    SEND: сообщение копируется в буфер, управление передаётся планировщику
    когда адресат, получив сообщение, делает вызов REPLY -- управление возвращается
    -----------------------------------------------------------------------------
  */
  case SEND:
    send((struct message *)arg);
    break;

  case REPLY:
    reply((struct message *)arg);
    break;

  default:
    break;
  }
}
