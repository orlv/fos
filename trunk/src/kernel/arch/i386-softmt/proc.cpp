/*
  Copyright (C) 2007-2008 Oleg Fedorov
 */


#include "include/context.h"
#include <fos/system.h>
#include <fos/fos.h>
#include <fos/mmu.h>

static bool active_tss_descriptor = 0;

void set_initial_task(context_t *context)
{
  system->gdt->load_tss(SEL_N(BASE_TSK_SEL), &context->descr);
  ltr(BASE_TSK_SEL);
  lldt(0);
}

void switch_context(context_t *current, context_t *next)
{
  if(active_tss_descriptor) {
    active_tss_descriptor = 0;
    system->gdt->load_tss(SEL_N(BASE_TSK_SEL), &next->descr);
    __asm__ __volatile__("ljmp $0x38, $0");
  } else {
    active_tss_descriptor = 1;
    system->gdt->load_tss(SEL_N(BASE_TSK_SEL) + 1, &next->descr);
    __asm__ __volatile__("ljmp $0x40, $0");
  }
}

void setup_context(context_t *context,
		   u32_t *pagedir,
		   off_t eip,
		   void *kernel_stack,
		   void *user_stack,
		   u16_t code_segment,
		   u16_t data_segment)
{
  context->tss = (struct TSS *)kmalloc(sizeof(TSS));

  context->tss->cr3 = (u32_t)kmem_phys_addr(PAGE((u32_t)pagedir));
  context->tss->eip = eip;

  context->tss->eflags = X86_EFLAGS_IOPL|X86_EFLAGS_IF|X86_EFLAGS;

  context->stack_pl0 = (off_t) kernel_stack;
  context->tss->esp0 = (off_t) kernel_stack + STACK_SIZE - 4;
  context->tss->esp = context->tss->ebp = (off_t) user_stack + STACK_SIZE - 4;
  context->tss->cs = (u16_t) code_segment;
  context->tss->es = (u16_t) data_segment;
  context->tss->ss = (u16_t) data_segment;
  context->tss->ds = (u16_t) data_segment;
  context->tss->ss0 = KERNEL_DATA_SEGMENT;
  
  context->tss->io_bitmap_base = 0xffff;

  /* создадим селектор TSS */
  system->gdt->set_tss_descriptor((off_t) context->tss, &context->descr);
}

int sched_ready()
{
  return (!active_tss_descriptor && (str() == 0x38)) || (active_tss_descriptor && (str() == 0x40));
}
