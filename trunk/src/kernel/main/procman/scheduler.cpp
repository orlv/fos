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

    system->cli();
    /* если поток только что приостановился в ожидании сообщения,
       и сообщения уже пришли — активируем */
    if((current_thread->state | TSTATE_WAIT_ON_RECV) && curr->item->signals_cnt.value()){
      current_thread->start(TSTATE_WAIT_ON_RECV);
    }
    system->sti();

    /* поток активен → перемещаем в конец списка активных задач */
    if(!current_thread->state && (curr != task.active)){
      curr->move_tail(task.active);
    }

    curr = task.active->next;
    current_thread = curr->item;
    //printk("[%s]\n", curr->item->process->name);
    
    /* если установлен и истек таймер -- отправляем сигнал */
    if(curr->item->alarm.time && curr->item->alarm.time <= _uptime){
      curr->item->alarm.time = 0;
      system->cli();
      curr->item->put_signal(0, SIGNAL_ALARM);
      system->sti();
    }

    /* если пришли сигналы -- отправляем соответствующие сообщения */
    if(curr->item->signals_cnt.value()){
      system->cli();
      curr->item->parse_signals();
      system->sti();
    }
      
    /*
     * Переключимся на выбранный процесс
     */
    curr->item->run();
    /*
     * Если мы здесь - значит произошло вытеснение процесса и переключение на планировщик
     */

    if(!preempt_status()){
      extern atomic_t preempt_count;
      preempt_count.set(0);
    }
  }
}
