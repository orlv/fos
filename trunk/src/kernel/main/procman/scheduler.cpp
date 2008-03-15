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

void sched_yield()
{
  preempt_reset();
  system->procman->scheduler();
  preempt_enable();
}

void TProcMan::scheduler()
{
  u32_t _uptime = kuptime();
  timer.check(_uptime);

  prev = curr;

  system->cli();
  /* если поток только что приостановился в ожидании сообщения,
     и сообщения уже пришли — активируем */
  if((curr->item->state | TSTATE_WAIT_ON_RECV) && curr->item->signals_cnt){
    curr->item->start(TSTATE_WAIT_ON_RECV);
  }
  system->sti();

  /* поток активен → перемещаем в конец списка активных задач */
  if(!curr->item->state && (curr != task.active)){
    curr->move_tail(task.active);
  }

  curr = task.active->next;
  //printk("[%s]\n", curr->item->process->name);
    
  /* если установлен и истек таймер -- отправляем сигнал */
  if(curr->item->alarm.time && curr->item->alarm.time <= _uptime){
    curr->item->alarm.time = 0;
    system->cli();
    curr->item->put_signal(0, SIGNAL_ALARM);
    system->sti();
  }

  /* если пришли сигналы -- отправляем соответствующие сообщения */
  if(curr->item->signals_cnt){
    system->cli();
    curr->item->parse_signals();
    system->sti();
  }
      
  /*
   * Переключимся на выбранный процесс
   */
  if(curr != prev)
    switch_context(&prev->item->context, &curr->item->context);

  /*if(_tss) {
      _tss = 0;
      system->gdt->load_tss(SEL_N(BASE_TSK_SEL), &curr->item->descr);
      __asm__ __volatile__("ljmp $0x38, $0");
    } else {
      _tss = 1;
      system->gdt->load_tss(SEL_N(BASE_TSK_SEL) + 1, &curr->item->descr);
      __asm__ __volatile__("ljmp $0x40, $0");
      }*/
  
}

