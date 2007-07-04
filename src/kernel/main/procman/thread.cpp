/*
  kernel/main/procman/thread.cpp
  Copyright (C) 2005-2007 Oleg Fedorov
*/

#include <process.h>
#include <procman.h>
#include <mm.h>
#include <mmu.h>
#include <system.h>
#include <hal.h>
#include <string.h>
#include <stdio.h>

Thread::Thread(class TProcess *process,
	       off_t eip,
	       u16_t flags,
	       void * kernel_stack,
	       void * user_stack,
	       u16_t code_segment,
	       u16_t data_segment)
{
  kmessage *_msg = new(kmessage);
  new_messages = new List<kmessage *>(_msg);      /* пустое сообщение */
  received_messages = new List<kmessage *>(_msg); /* пустое сообщение */
  
  this->process = process;

  set_tss(eip, kernel_stack, user_stack, code_segment, data_segment);

  this->flags = flags;
}

Thread::~Thread()
{
  List<kmessage *> *curr, *n;
  /* удалим все сообщения */
#warning вернуть ошибку отправителям
  list_for_each_safe(curr, n, new_messages){
    delete curr->item;
    delete curr;
  }

  delete new_messages->item;
  delete new_messages;

  kfree((void *)stack_pl0);
  delete tss;
}

void Thread::set_tss(register off_t eip,
		     register void *kernel_stack,
		     register void *user_stack,
		     u16_t code_segment,
		     u16_t data_segment)
{
  tss = (struct TSS *)kmalloc(sizeof(TSS));

  tss->cr3 = (u32_t)kmem_phys_addr(PAGE((u32_t)process->memory->pagedir));
  tss->eip = eip;

  tss->eflags = X86_EFLAGS_IOPL|X86_EFLAGS_IF|X86_EFLAGS;

  stack_pl0 = (off_t) kernel_stack;
  tss->esp0 = (off_t) kernel_stack + STACK_SIZE - 1;
  tss->esp = tss->ebp = (off_t) user_stack + STACK_SIZE - 1;
  tss->cs = (u16_t) code_segment;
  tss->es = (u16_t) data_segment;
  tss->ss = (u16_t) data_segment;
  tss->ds = (u16_t) data_segment;
  tss->ss0 = KERNEL_DATA_SEGMENT;
  
  tss->io_bitmap_base = 0xffff;

  /* создадим селектор TSS */
  hal->gdt->set_tss_descriptor((off_t) tss, &descr);
}

void Thread::run()
{
  hal->gdt->load_tss(SEL_N(BASE_TSK_SEL), &descr);
  __asm__ __volatile__("ljmp $0x38, $0");
}
