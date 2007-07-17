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

extern Timer *SysTimer;

void sched_srv()
{
  hal->procman->scheduler();
}

void TProcMan::scheduler()
{
  List<Thread *> *curr = threadlist;
  u32_t _uptime;

  while (1) {
    asm("incb 0xb8000+158\n" "movb $0x5f,0xb8000+159");

    _uptime = uptime();
    
    /* Выбираем следующий подходящий для запуска поток */
    do {
      curr = curr->next;

      /* если установлен и истек таймер -- отправляем сигнал */
      if(curr->item->get_alarm() && curr->item->get_alarm() < _uptime){
	//printk("alarm = 0x%X, uptime=0x%X \n", curr->item->get_alarm(), _uptime);
	curr->item->set_alarm(0);
	curr->item->set_signal(SIGNAL_ALARM);
      }

      /* если пришли сигналы -- отправляем соответствующие сообщения */
      if(curr->item->get_signals()){
	curr->item->parse_signals();
      }
      
      /* Процесс готов к запуску? */
      if ((curr->item->flags & FLAG_TSK_READY) &&
	  !(curr->item->flags & (FLAG_TSK_SEND | FLAG_TSK_RECV)))
	break;
    } while (1);

    current_thread = curr->item;

    /*
     * Переключимся на выбранный процесс
     */
    curr->item->run();
    /*
     * Если мы здесь - значит произошло вытеснение процесса и переключение на планировщик
     */
  }
}
