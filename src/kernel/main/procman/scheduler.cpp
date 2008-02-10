/*
  kernel/main/procman/scheduler.cpp
  Copyright (C) 2005-2007 Oleg Fedorov
*/

#include <fos/mm.h>
#include <fos/procman.h>
#include <fos/printk.h>
#include <fos/fos.h>
#include <fos/drivers/char/timer/timer.h>
#include <fos/fs.h>
#include <sys/elf32.h>

extern Timer *SysTimer;

void sched_srv()
{
  system->procman->scheduler();
}

void TProcMan::scheduler()
{
  List<Thread *> *curr = task.active;
  current_thread = curr->item;
  u32_t _uptime;

  while (1) {
    _uptime = kuptime();
    timer.check(_uptime);
    /* Выбираем следующий подходящий для запуска поток */
    //do {
      /*      if(((curr->item->flags & FLAG_TSK_TERM) || (curr->item->flags & FLAG_TSK_EXIT_THREAD)) && !(curr->item->flags & FLAG_TSK_SYSCALL)){
	curr->item->flags &= ~FLAG_TSK_READY;
	curr = do_kill(curr);
	continue;
	} else */

    system->cli();
    if(current_thread->wstate && !current_thread->wflag){
      current_thread->wstate = 0;
      activate(current_thread->me);
    }
    system->sti();
    
    if(!current_thread->wstate && (curr != task.active)){
      curr->move_tail(task.active);
    }

    curr = task.active->next;
    current_thread = curr->item;
    //printk("[%s]\n", curr->item->process->name);
    
    /* если установлен и истек таймер -- отправляем сигнал */
    /*    if(curr->item->alarm.get() && curr->item->alarm.get() < _uptime){
      curr->item->alarm.set(0);
      curr->item->put_signal(0, SIGNAL_ALARM);
      }*/
    if(curr->item->alarm.time && curr->item->alarm.time <= _uptime){
      curr->item->alarm.time = 0;
      curr->item->put_signal(0, SIGNAL_ALARM);
    }

    /* если пришли сигналы -- отправляем соответствующие сообщения */
    if(curr->item->signals_cnt.value()){
      curr->item->parse_signals();
    }
      
      /* Процесс готов к запуску? */
      /*      if ((curr->item->flags & FLAG_TSK_READY) &&
	  !((curr->item->flags & FLAG_TSK_SEND) || (curr->item->flags & FLAG_TSK_RECV)))
	break;
	} while (1);*/
    //printk("[%s]\n", curr->item->process->name);
    /*
     * Переключимся на выбранный процесс
     */
    curr->item->run();
    /*
     * Если мы здесь - значит произошло вытеснение процесса и переключение на планировщик
     */
  }
}
