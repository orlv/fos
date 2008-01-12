/*
  kernel/main/procman/thread.cpp
  Copyright (C) 2005-2007 Oleg Fedorov
*/

#include <fos/thread.h>
#include <fos/mmu.h>
#include <fos/fos.h>
#include <fos/pager.h>
#include <fos/printk.h>

Thread::Thread(class TProcess *process, off_t eip, u16_t flags, void * kernel_stack, void * user_stack, u16_t code_segment, u16_t data_segment)
{
  //kmessage *_msg = new(kmessage);
  //  new_messages = new List<kmessage *>(_msg);      /* пустое сообщение */
  //  received_messages = new List<kmessage *>(_msg); /* пустое сообщение */
  
  this->process = process;

  set_tss(eip, kernel_stack, user_stack, code_segment, data_segment);

  this->flags = flags;
}

Thread::~Thread()
{
  for(int n=0; n<256; n++){
    if(system->user_int_handler[n] == this){
      system->pic->mask(n);
      system->user_int_handler[n] = 0;
    }
  }
  List<kmessage *> *curr, *n;
  /* удалим все сообщения, вернем ошибки отправителям */
  list_for_each_safe(curr, n, (&messages.unread.list)){
    curr->item->reply_size = 0;
    curr->item->thread->flags &= ~FLAG_TSK_SEND;
    delete curr->item;
    delete curr;
  }
  //delete new_messages->item;
  //delete new_messages;
  list_for_each_safe(curr, n, (&messages.read.list)){
    curr->item->reply_size = 0;
    curr->item->thread->flags &= ~FLAG_TSK_SEND;
    delete curr->item;
    delete curr;
  }
  //delete received_messages;
  kfree((void *)stack_pl0, STACK_SIZE);
  kfree((void *)tss, sizeof(TSS));
}

void Thread::set_tss(register off_t eip,
		     register void *kernel_stack,
		     register void *user_stack,
		     u16_t code_segment,
		     u16_t data_segment)
{
  tss = (struct TSS *)kmalloc(sizeof(TSS));

  tss->cr3 = (u32_t)kmem_phys_addr(PAGE((u32_t)process->memory->pager->pagedir));
  tss->eip = eip;

  tss->eflags = X86_EFLAGS_IOPL|X86_EFLAGS_IF|X86_EFLAGS;

  stack_pl0 = (off_t) kernel_stack;
  tss->esp0 = (off_t) kernel_stack + STACK_SIZE - 4;
  tss->esp = tss->ebp = (off_t) user_stack + STACK_SIZE - 4;
  tss->cs = (u16_t) code_segment;
  tss->es = (u16_t) data_segment;
  tss->ss = (u16_t) data_segment;
  tss->ds = (u16_t) data_segment;
  tss->ss0 = KERNEL_DATA_SEGMENT;
  
  tss->io_bitmap_base = 0xffff;

  /* создадим селектор TSS */
  system->gdt->set_tss_descriptor((off_t) tss, &descr);
}

void Thread::run()
{
  system->gdt->load_tss(SEL_N(BASE_TSK_SEL), &descr);
  __asm__ __volatile__("ljmp $0x38, $0");
}

res_t Thread::put_message(kmessage *message)
{
  system->mt_disable();
  if(messages.unread.count.value() >= MAX_MSG_COUNT){
    system->mt_enable();
    return RES_FAULT2;
  }

  messages.unread.list.add_tail(message);
  messages.unread.count.inc();
  flags &= ~FLAG_TSK_RECV;	/* сбросим флаг ожидания получения сообщения (если он там есть) */

  system->mt_enable();
  return RES_SUCCESS;
}

void Thread::parse_signals()
{
  for(u32_t n=0, mask=1; signals && (n < sizeof(this->signals)*8); n++, mask = mask << 1){
    if(signals & mask){
      kmessage *message = new kmessage;
      message->size = 0;
      message->arg[0] = n;
      message->flags = MESSAGE_ASYNC;
      message->thread = THREAD(_MSG_SENDER_SIGNAL);
      this->put_message(message);
      signals = signals & ~mask;
    }
  } 
}
