/*
  kernel/main/procman/scheduler.cpp
  Copyright (C) 2005-2007 Oleg Fedorov
*/

#include <mm.h>
#include <procman.h>
#include <stdio.h>
#include <system.h>
#include <drivers/char/timer/timer.h>
#include <hal.h>
#include <fs.h>
#include <elf32.h>

asmlinkage void scheduler_starter();
extern TTime *SysTimer;

/*
 * Для флоппи
 * TODO: Убрать 
 */
extern u8_t motor;		/* Отображает состояние мотора 0-выключен, 1-включен */
extern u16_t mtick;		/* Больше нуля - то уменьшается на 1, когда станет равным 0 - мотор выключается */
extern u16_t tmout;		/* Просто таймер, его небходимо будет убрать после добавления такой функции в ядро */

void start_sched()
{
  hal->ProcMan->scheduler();
}

void TProcMan::scheduler()
{
  volatile List *curr = proclist;
  TProcess *process;
  while (1) {
#warning *** TODO: scheduler(): сделать таймеры

    asm("incb 0xb8000+158\n" "movb $0x5e,0xb8000+159");

#if 0
    /* Вместо этого кода сделать таймеры */
    if (tmout)
      tmout--;
    if (mtick > 0)
      mtick--;
    else if (motor) {
      outportb(0x3f2, 0x0c);	/* Выключим мотор флоппи */
      motor = 0;
    }
#endif
    /* Выбираем следующий подходящий для запуска процесс */
    do {
      curr = curr->next;
      process = (TProcess *) curr->data;
#if 0
      if (process->flags & FLAG_TSK_TERM) {
	curr = curr->next;
	kill(process->pid);
	continue;
      }
#endif
      /* Процесс готов к запуску? */
      if ((process->flags & FLAG_TSK_READY) &&
	  !(process->flags & (FLAG_TSK_SEND | FLAG_TSK_RECV)))
	break;
    } while (1);

    CurrentProcess = process;
    /* Переключимся на выбранный процесс */
    process->run();
    /*
     * Ring
     */
  }
}
