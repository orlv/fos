/*
  kernel/main/ints.cpp
  Copyright (C) 2004-2007 Oleg Fedorov
*/

#include <fos/traps.h>
#include <fos/printk.h>
#include <fos/fos.h>
#include <fos/mm.h>
#include <fos/procman.h>

asmlinkage void sys_call_handler();

void dump_stack(u32_t ebp)
{
  u32_t *s;
  if((ebp + 36*sizeof(u32_t)) < (ebp|0xfff))
    s = (u32_t *) (ebp + 36*sizeof(u32_t));
    else
  s = (u32_t *) ((ebp | 0xfff) - 3);

  printk("Memory dump from 0x%08X to ebp=0x%08X:\n", (u32_t)s, ebp);
  int n=0;
  while((u32_t)s > ebp) {
    s--;
    printk("0x%08X ", *s);
    n++; if(n==4) { n=0; printk("\n"); }
  }
  printk("\n");
}

void exception(const char *str, unsigned int cs,  unsigned int address, unsigned int errorcode, unsigned int ebp)
{
#if 0
  printk("\n--------------------------------------------------------------------------------" \
	 "Exception: %s \n"						\
	 "At addr: 0x%02X:0x%08X\n"					\
	 "Errorcode: 0x%X",
	 str, cs, address, errorcode);

  system->panic("fault in kernel mode!");
#endif

  printk("\n-------------------------------------------------------------------------------\n"
	 "Exception: %s \n"
	 "At addr: 0x%02X:0x%08X\n"
	 "Thread: 0x%X, Process: 0x%X \n"
	 "Name: [%s]\n"
	 "Errorcode: 0x%X\n"
	 "-------------------------------------------------------------------------------", 
	 str, cs, address, system->procman->curr->item, system->procman->curr->item->process, system->procman->curr->item->process->name, errorcode);
  
  if((system->procman->curr->item->flags & FLAG_TSK_KERN) ||
     (address < USER_MEM_BASE)){
    system->panic("fault in kernel mode!");
  } else {
    printk("\nfault in user task! task terminated\n");
    dump_stack(ebp);
    system->procman->curr->item->flags |= FLAG_TSK_TERM;
    system->procman->curr->item->deactivate();
    while(1) sched_yield();
  }
}

EXCEPTION_HANDLER2(divide_error_exception)
{
  exception("[0x00] Divide Error", cs, address, 0, ebp);
}

EXCEPTION_HANDLER(debug_exception)
{
  exception("[0x01] Debug", cs, address, errorcode, ebp);
}

EXCEPTION_HANDLER(NMI_exception)
{
  exception("[0x02] NMI", cs, address, errorcode, ebp);
}

EXCEPTION_HANDLER(int3_exception)
{
  /* NONE */
  //exception("int3", cs, address, errorcode, ebp);
}

EXCEPTION_HANDLER(overflow_exception)
{
  exception("[0x04] overflow", cs, address, errorcode, ebp);
}

EXCEPTION_HANDLER(bound_exception)
{
  exception("[0x05] bound", cs, address, errorcode, ebp);
}

EXCEPTION_HANDLER(invalid_operation_exception)
{
  exception("[0x06] invalid operation", cs, address, errorcode, ebp);
}

EXCEPTION_HANDLER(FPU_not_present_exception)
{
  exception("[0x07] FPU not present", cs, address, errorcode, ebp);
}

EXCEPTION_HANDLER(double_fault_exception)
{
  exception("[0x08] double fault", cs, address, errorcode, ebp);
}

EXCEPTION_HANDLER(reserved_exception)
{
  exception("reserved interrupt", cs, address, errorcode, ebp);
}

EXCEPTION_HANDLER(invalid_TSS_exception)
{
  exception("[0x0A] invalid TSS", cs, address, errorcode, ebp);
}

EXCEPTION_HANDLER(segment_not_present_exception)
{
  exception("[0x0B] segment not present", cs, address, errorcode, ebp);
}

EXCEPTION_HANDLER(stack_fault_exception)
{
  exception("[0x0C] stack fault", cs, address, errorcode, ebp);
}

EXCEPTION_HANDLER(general_protection_fault_exception)
{
  exception("[0x0D] general protection fault", cs, address, errorcode, ebp);
}

EXCEPTION_HANDLER(page_fault_exception)
{
  u32_t cr2;
  asm volatile ("mov %%cr2, %%eax":"=a" (cr2));
  printk("\n-------------------------------------------------------------------------------\n");
  int level;
  if (errorcode & 4)
    level = 3;
  else
    level = 0;

  printk("[0x0E] page fault!\nCPL=%d\n", level);
  printk("cr2=0x%08X\n", cr2);

  if (errorcode & 2)
    printk("Write error: ");
  else
    printk("Read error: ");

  if (errorcode & 1)
    printk("permission denied");
  else
    printk("page not found");

  exception("[0x0E] page fault", cs, address, errorcode, ebp);
}

EXCEPTION_HANDLER(FPU_error_exception)
{
  exception("[0x10] FPU error", cs, address, errorcode, ebp);
}

EXCEPTION_HANDLER(align_error_exception)
{
  exception("[0x11] align error", cs, address, errorcode, ebp);
}

EXCEPTION_HANDLER(machine_depend_error_exception)
{
  exception("[0x12] machine depend error", cs, address, errorcode, ebp);
}


asm(".globl empty_interrupt \n"						\
    "empty_interrupt: \n"						\
    "mov $0x20, %al \n"							\
    "outb $0x20 \n"							\
    "iret");

asmlinkage void empty_interrupt();

#if 0
void common_interrupt(u8_t n)
{
  system->pic->mask(n); /* Демаскировку должен производить обработчик */
  if(system->user_int_handler[n]){
    system->user_int_handler[n]->put_signal(n, SIGNAL_IRQ);
  } else if(n == 7 || n == 15) {
	printk("Spurious interrupt %u\n", n);
  } else 
    system->panic("Unhandled interrupt received!\n");

  if(n>7)
    system->outportb(0xa0, 0x20);
  system->outportb(0x20, 0x20);
}

IRQ_HANDLER(irq_0)
{
  extern Timer *SysTimer;
  SysTimer->tick(); /* Считаем время */

  asm("incb 0xb8000+150\n" "movb $0x5e,0xb8000+151 ");

  //bool _tss = system->procman->_tss;
  if (!system->preempt.status() || !sched_ready()) {
      //((!_tss && (str() != 0x38)) || (_tss && (str() != 0x40)))) {
    system->outportb(0x20, 0x20);
    return;
  }

  system->outportb(0x20, 0x20);
  sched_yield();
}

IRQ_HANDLER(irq_1)  { common_interrupt(1);  }

IRQ_HANDLER(irq_3)  { common_interrupt(3);  }
IRQ_HANDLER(irq_4)  { common_interrupt(4);  }
IRQ_HANDLER(irq_5)  { common_interrupt(5);  }
IRQ_HANDLER(irq_6)  { common_interrupt(6);  }
IRQ_HANDLER(irq_7)  { common_interrupt(7);  }
IRQ_HANDLER(irq_8)  { common_interrupt(8);  }
IRQ_HANDLER(irq_9)  { common_interrupt(9);  }
IRQ_HANDLER(irq_10) { common_interrupt(10); }
IRQ_HANDLER(irq_11) { common_interrupt(11); }
IRQ_HANDLER(irq_12) { common_interrupt(12); }
IRQ_HANDLER(irq_13) { common_interrupt(13); }
IRQ_HANDLER(irq_14) { common_interrupt(14); }
IRQ_HANDLER(irq_15) { common_interrupt(15); }

#endif

asmlinkage void common_interrupt(u32_t vector, u32_t cs, u32_t address) {
	system->ic->Route(vector);
}

extern "C" {
	extern char interrupt_vectors_start[];
	extern char interrupt_vectors_end[];
}

void setup_idt()
{
  u16_t i;

  system->idt->set_trap_gate(0x00, (off_t) & divide_error_exception, 0);
  system->idt->set_trap_gate(0x01, (off_t) & debug_exception, 0);
  system->idt->set_trap_gate(0x02, (off_t) & NMI_exception, 0);
  system->idt->set_trap_gate(0x03, (off_t) & int3_exception, 3);	/* прерывания 3-5 могут вызываться из задач */
  system->idt->set_trap_gate(0x04, (off_t) & overflow_exception, 3);
  system->idt->set_trap_gate(0x05, (off_t) & bound_exception, 3);
  system->idt->set_trap_gate(0x06, (off_t) & invalid_operation_exception, 0);
  system->idt->set_trap_gate(0x07, (off_t) & FPU_not_present_exception, 0);
  system->idt->set_trap_gate(0x08, (off_t) & double_fault_exception, 0);
  system->idt->set_trap_gate(0x09, (off_t) & reserved_exception, 0);
  system->idt->set_trap_gate(0x0A, (off_t) & invalid_TSS_exception, 0);
  system->idt->set_trap_gate(0x0B, (off_t) & segment_not_present_exception, 0);
  system->idt->set_trap_gate(0x0D, (off_t) & general_protection_fault_exception, 0);
  system->idt->set_trap_gate(0x0E, (off_t) & page_fault_exception, 0);
  system->idt->set_trap_gate(0x0F, (off_t) & reserved_exception, 0);
  system->idt->set_trap_gate(0x10, (off_t) & FPU_error_exception, 0);
  system->idt->set_trap_gate(0x11, (off_t) & align_error_exception, 0);
  system->idt->set_trap_gate(0x12, (off_t) & machine_depend_error_exception, 0);

  for (i = 0x13; i < 0x20; i++)
    system->idt->set_trap_gate(i, (off_t) & reserved_exception, 0);

  size_t one_vector = ((u32_t)interrupt_vectors_end - (u32_t)interrupt_vectors_start) / 221;

  for(int i = 0; i <= 224; i++) {
    system->idt->set_intr_gate(0x20 + i, (off_t) interrupt_vectors_start + i * one_vector);
  }

#if 0
  system->idt->set_intr_gate(0x20, (off_t) & irq_0); /* timer */
  system->idt->set_intr_gate(0x21, (off_t) & irq_1); /* keyboard */

  system->idt->set_intr_gate(0x23, (off_t) & irq_3);
  system->idt->set_intr_gate(0x24, (off_t) & irq_4);
  system->idt->set_intr_gate(0x25, (off_t) & irq_5);
  system->idt->set_intr_gate(0x26, (off_t) & irq_6); /* floppy */
  system->idt->set_intr_gate(0x27, (off_t) & irq_7);
  system->idt->set_intr_gate(0x28, (off_t) & irq_8);
  system->idt->set_intr_gate(0x29, (off_t) & irq_9);
  system->idt->set_intr_gate(0x2a, (off_t) & irq_10);
  system->idt->set_intr_gate(0x2b, (off_t) & irq_11);
  system->idt->set_intr_gate(0x2c, (off_t) & irq_12);
  system->idt->set_intr_gate(0x2d, (off_t) & irq_13);
  system->idt->set_intr_gate(0x2e, (off_t) & irq_14);
  system->idt->set_intr_gate(0x2f, (off_t) & irq_15);
#endif

  

  system->idt->set_trap_gate(0xFD, (off_t) & sys_call_handler, 3); /* системный вызов */

//  for (i = 0x31; i < 0x100; i++)
//    system->idt->set_trap_gate(i, (off_t) & empty_interrupt, 0);
}
