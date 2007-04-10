/*
 * kernel/main/syscall.cpp
 * Copyright (C) 2005-2006 Oleg Fedorov
 */

#include <system.h>
#include <stdio.h>
#include <tasks.h>
#include <string.h>

void putpixel(u32_t offs, u16_t code);

#define SEND    1
#define RECEIVE 2
#define REPLY   3
#define MEM_ALLOC 4
#define MEM_MAP 5

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

asmlinkage u32_t sys_call(u32_t arg1, u32_t arg2)
{
  extern TProcess *CurrentProcess;

  struct message *message;
  struct message *msg;
  size_t size;
  TProcess *p;

  //printk("\nSyscall #%d, Process %d ", arg1,  CurrentProcess->pid);
  
  switch (arg1) {
  case MEM_ALLOC:
    *(u32_t *) arg2 = (u32_t) CurrentProcess->mem_alloc(*(u32_t *) arg2);
    break;

  case MEM_MAP:
    struct memmap *mmp;
    mmp = (struct memmap *)arg2;
    mmp->ptr = (u32_t) CurrentProcess->mem_alloc(mmp->ptr, mmp->size);
    break;
    /*
       case PRINT:
       cli();
       printk("%c", arg2);
       sti();
       break;
     */
  case RECEIVE:
    /* если нет ни одного входящего сообщения, отключаемся в ожидании */
    if (CurrentProcess->msg->empty()) {
      CurrentProcess->flags |= FLAG_TSK_RECV;
      pause();
    }

    /* SEND-->> */
#warning ЗАМЕНИТЬ cli() на мьютекс!
    cli();
    msg = (struct message *)CurrentProcess->msg->next->data;
    message = (struct message *)arg2;

    if (msg->send_size > message->recv_size)
      size = message->recv_size;
    else
      size = msg->send_size;

    if (size) {
      /* копируем сообщение из ядра в процесс */
      memcpy((u32_t *) message->recv_buf, (u32_t *) msg->send_buf, size);
    }

    /* отметим, сколько байт было передано */
    msg->send_size = message->recv_size = size;

    delete(u32_t *) msg->send_buf;
    sti();
    break;

    /*
       ---------------------------------------------
       SEND: сообщение копируется в буфер, управление передаётся планировщику
       когда адресат, получив сообщение, делает вызов REPLY -- управление возвращается
       ---------------------------------------------
     */
  case SEND:
    message = (struct message *)arg2;
    extern TProcMan *ProcMan;

    if (!(p = ProcMan->get_process_by_pid(message->pid)))
      break;

    /* скопируем сообщение в память ядра */
    msg = new(struct message);
    msg->send_buf = new char[message->send_size];
    msg->send_size = message->send_size;
    memcpy((u32_t *) msg->send_buf, (u32_t *) message->send_buf,
	   msg->send_size);

    msg->recv_buf = 0;
    msg->recv_size = message->recv_size;
    msg->pid = (u32_t) CurrentProcess;

#warning ЗАМЕНИТЬ cli() на мьютекс!
    cli();
    p->msg->add_tail(msg);

    p->flags &= ~FLAG_TSK_RECV;	/* сбросим флаг ожидания получения сообщения (если он там есть) */
    CurrentProcess->flags |= FLAG_TSK_SEND;	/* ожидаем ответа */
    sti();
    pause();

    /* REPLY-->> */
    memcpy((u32_t *) message->recv_buf, (u32_t *) msg->recv_buf,
	   msg->recv_size);
    message->send_size = msg->send_size;	/* сколько байт дошло до получателя */
    message->recv_size = msg->recv_size;	/* сколько байт ответа пришло */

    delete(u32_t *) msg->recv_buf;
    delete msg;

    break;

  case REPLY:
    msg = (struct message *)CurrentProcess->msg->next->data;
    message = (struct message *)arg2;

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
    cli();
    ((TProcess *) msg->pid)->flags &= ~FLAG_TSK_SEND;	/* сбросим флаг SEND */
    delete CurrentProcess->msg->next;
    sti();

    //pause();

    break;

  default:
    break;
  }

  return 0;
}
