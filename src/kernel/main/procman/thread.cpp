/*
  kernel/main/procman/thread.cpp
  Copyright (C) 2005-2007 Oleg Fedorov
*/

#include <fos/thread.h>
#include <fos/mmu.h>
#include <fos/fos.h>
#include <fos/pager.h>
#include <fos/printk.h>

Thread::Thread(class TProcess *process, off_t eip, u16_t flags, void * kernel_stack, void * user_stack, u16_t code_segment, u16_t data_segment):messages(this)
{
  
  this->process = process;
  setup_context(&context, process->memory->pager->pagedir, eip, kernel_stack, user_stack, code_segment, data_segment);
  //set_tss(eip, kernel_stack, user_stack, code_segment, data_segment);
  this->flags = flags;
}

Thread::~Thread()
{
  printk("Terminating threads not implemented!\n");
#if 0
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

  list_for_each_safe(curr, n, (&messages.read.list)){
    curr->item->reply_size = 0;
    curr->item->thread->flags &= ~FLAG_TSK_SEND;
    delete curr->item;
    delete curr;
  }

  kfree((void *)stack_pl0, STACK_SIZE);
  kfree((void *)tss, sizeof(TSS));
#endif
}

#if 0
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
#endif

void Thread::activate()
{
  system->procman->activate(me);
}

void Thread::deactivate()
{
  system->procman->stop(me);
}

void Thread::parse_signals()
{
  List<signal *> *curr, *n;

  list_for_each_safe(curr, n, (&signals)) {
    //printk("[%s] %d %d\n", process->name, curr->item->n, curr->item->data);
    kmessage *message = new kmessage;
    message->size = 0;
    message->arg[0] = curr->item->n;
    message->arg[1] = curr->item->data;
    message->flags = MSG_ASYNC;
    message->thread = 0;
    messages.put_message(message);
    delete curr->item;
    delete curr;
    signals_cnt--;
  }
}
