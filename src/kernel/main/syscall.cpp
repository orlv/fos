/*
 * kernel/main/syscall.cpp
 * Copyright (C) 2005-2007 Oleg Fedorov
 */

#include <fos/printk.h>
#include <fos/fos.h>
#include <fos/syscall.h>
#include <fos/pager.h>
#include <fos/drivers/interfaces/timer.h>
#include <fos/ipc.h>
#include <string.h>

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

u32_t uptime();

void syscall_enter()
{
  system->procman->curr->item->flags |= FLAG_TSK_SYSCALL;
}

void syscall_exit()
{
  //preempt_on();
  system->procman->curr->item->flags &= ~FLAG_TSK_SYSCALL;
  if((system->procman->curr->item->flags & FLAG_TSK_TERM) || (system->procman->curr->item->flags & FLAG_TSK_EXIT_THREAD)) {
#warning см. сюда
    //system->procman->curr->item->flags &= ~FLAG_TSK_READY;
    sched_yield();
  }
}

SYSCALL_HANDLER(sys_call_handler)
{
  //printk("Syscall #%d (%s) arg1=0x%X, arg2=0x%X \n", cmd,  system->procman->curr->item->process->name, arg1, arg2);
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
    system->ic->mask(arg1);
    break;

  case _FOS_UNMASK_INTERRUPT:
    system->ic->unmask(arg1);
    break;

  case _FOS_SCHED_YIELD:
    sched_yield();
    result = 0;
    break;

  case _FOS_UPTIME:
    result = kuptime();
    break;

  case _FOS_ALARM:
    _uptime = kuptime();
    if(system->procman->curr->item->alarm.time > _uptime)
      result = system->procman->curr->item->alarm.time - _uptime;
    else
      result = 0;

    if(arg1)  /* текущее время + arg1 */
      system->procman->timer.add(system->procman->curr->item, _uptime + arg1);
    else
      system->procman->timer.add(system->procman->curr->item, arg2);
    break;

  case _FOS_MYTID:
    result = system->procman->curr->item->tid;
    break;

  case _FOS_GET_PAGE_PHYS_ADDR:
    if(arg1 > USER_MEM_BASE)
      result = OFFSET(phys_addr_from(PAGE(arg1), system->procman->curr->item->process->
				     memory->pager->pagedir));
    else
      result = 0;
    break;

  default:
    result = RES_FAULT;
    break;
  }

  syscall_exit(); /* проверим, не убили ли нас ;) */
  return result;
}
