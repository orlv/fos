/*
	kernel/main/exception/traps.cpp
	Copyright (C) 2004-2006 Oleg Fedorov
*/

#include <traps.h>
#include <stdio.h>
#include <system.h>
#include <mm.h>
#include <dt.h>
#include <tasks.h>

/*
	Из файла kernel/exceprion/traps.s:
*/

asmlinkage void timer_handler_wrapper();
asmlinkage void floppy_handler_wrapper();
asmlinkage void keyboard_handler_wrapper();

asmlinkage void divide_error_trap();
asmlinkage void debug_trap();
asmlinkage void NMI_trap();
asmlinkage void int3();
asmlinkage void overflow();
asmlinkage void BR();
asmlinkage void invalid_operation_trap();
asmlinkage void FPU_not_present_trap();
asmlinkage void double_fault_trap();
asmlinkage void reserved_trap();
asmlinkage void invalid_TSS_trap();
asmlinkage void segment_not_present_trap();
asmlinkage void stack_fault_trap();
asmlinkage void general_protection_fault_trap();
asmlinkage void page_fault_trap();
asmlinkage void FPU_error_trap();
asmlinkage void align_error_trap();
asmlinkage void machine_depend_error_trap();

asmlinkage void interrupt_hdl_not_present_trap();

asmlinkage void trap_gate();

volatile extern TProcess *CurrentProcess;

void halt()
{
  cli();
  while (1)
    hlt();
}

void panic(string str)
{
  cli();

  printk
      ("\n--------------------------------------------------------------------------------");
  printk("Kernel panic: %s \n", str);
  u16_t id = curPID();
  printk("ID: %d \n", id);
  if (CurrentProcess)
    printk("PID: %d \n", CurrentProcess->pid);
  printk("System Halted!\n");
  printk
      ("--------------------------------------------------------------------------------");
  halt();
}

void end_process(u32_t eip, u16_t cs)
{
  printk("\nEIP: 0x%04X:0x%08lX", cs, eip);
  panic("SUXX");
}

asmlinkage void divide_error_handler(u32_t eip, u16_t cs)
{
  end_process(eip, cs);
}

asmlinkage void debug_handler(u32_t eip, u16_t cs)
{
  printk
      ("\n-------------------------------------------------------------------------------");
  printk("\n[0x01] Debug");
  end_process(eip, cs);
}

asmlinkage void NMI_handler(u32_t eip, u16_t cs)
{
  printk
      ("\n-------------------------------------------------------------------------------");
  printk("\n[0x02] NMI");
  end_process(eip, cs);
}

asmlinkage void int3_handler(u32_t eip, u16_t cs)
{
  pause();
}

asmlinkage void overflow_handler(u32_t eip, u16_t cs)
{
  printk
      ("\n-------------------------------------------------------------------------------");
  printk("\n[0x04] overflow");
  end_process(eip, cs);
}

asmlinkage void bound_handler(u32_t eip, u16_t cs)
{
  printk
      ("\n-------------------------------------------------------------------------------");
  printk("\n[0x05] bound");
  end_process(eip, cs);
}

asmlinkage void invalid_operation_handler(u32_t eip, u16_t cs)
{
  printk
      ("\n-------------------------------------------------------------------------------");
  printk("\n[0x06] invalid operation");
  end_process(eip, cs);
}

asmlinkage void FPU_not_present_handler(u32_t eip, u16_t cs)
{
  printk
      ("\n-------------------------------------------------------------------------------");
  printk("\n[0x07] FPU not present");
  end_process(eip, cs);
}

asmlinkage void double_fault_handler(u16_t errorcode, u32_t eip, u16_t cs)
{
  printk
      ("\n-------------------------------------------------------------------------------");
  printk("\n[0x08] double fault");
  printk("\nErrorcode: [0x%X]", errorcode);
  end_process(eip, cs);
}

asmlinkage void reserved_handler(u32_t eip, u16_t cs)
{
  printk
      ("\n-------------------------------------------------------------------------------");
  printk("\nreserved interrupt");
  end_process(eip, cs);
}

asmlinkage void invalid_TSS_handler(u16_t errorcode, u32_t eip, u16_t cs)
{
  printk
      ("\n-------------------------------------------------------------------------------");
  printk("\n[0x0A] invalid TSS");
  printk("\nErrorcode: [0x%X]", errorcode);
  end_process(eip, cs);
}

asmlinkage void segment_not_present_handler(u16_t errorcode, u32_t eip,
					    u16_t cs)
{
  printk
      ("\n-------------------------------------------------------------------------------");
  printk("\n[0x0B] segment not present");
  printk("\nErrorcode: [0x%X]", errorcode);
  end_process(eip, cs);
}

asmlinkage void stack_fault_handler(u16_t errorcode, u32_t eip, u16_t cs)
{
  printk
      ("\n-------------------------------------------------------------------------------");
  printk("\n[0x0C] stack fault");
  printk("\nErrorcode: [0x%X]", errorcode);
  end_process(eip, cs);
}

asmlinkage void general_protection_fault_handler(u16_t errorcode, u32_t eip,
						 u32_t cs)
{
  printk
      ("\n-------------------------------------------------------------------------------");
  printk("\n[0x0D] general protection fault");
  printk("\nErrorcode: [0x%X]", errorcode);
  end_process(eip, cs);
}

asmlinkage void page_fault_handler(u16_t errorcode, u32_t eip, u16_t cs)
{
  u32_t cr2;
  asm volatile ("mov %%cr2, %%eax":"=a" (cr2));
  printk
      ("\n-------------------------------------------------------------------------------");
  printk("\n[0x0E] page fault");
  printk("\ncr2 = 0x%X", cr2);
  if (errorcode & 4)
    printk("\nLevel 3");
  else
    printk("\nLevel 0");
  if (errorcode & 2)
    printk("\nWrite error: ");
  else
    printk("\nRead error: ");
  if (errorcode & 1)
    printk("Permission denied");
  else
    printk("Page not found");
  end_process(eip, cs);
}

asmlinkage void FPU_error_handler(u32_t eip, u16_t cs)
{
  printk
      ("\n-------------------------------------------------------------------------------");
  printk("\n[0x10] FPU error");
  end_process(eip, cs);
}

asmlinkage void align_error_handler(u16_t errorcode, u32_t eip, u16_t cs)
{
  printk
      ("\n-------------------------------------------------------------------------------");
  printk("\nErrorcode: [0x%X]", errorcode);
  printk("\n[0x11] align error");
  end_process(eip, cs);
}

asmlinkage void machine_depend_error_handler()
{
  printk
      ("\n-------------------------------------------------------------------------------");
  printk("\n[0x12] machine depend error");
  halt();
}

asmlinkage void interrupt_hdl_not_present_handler()
{
  printk
      ("\n-------------------------------------------------------------------------------");
  printk("\n[0x??] Unknown interrupt");
  halt();
}

asmlinkage void setup_idt()
{
  u16_t i;
  extern DTMan *DTman;

  DTman->set_trap_gate(0x00, (off_t) & divide_error_trap, 0);
  DTman->set_trap_gate(0x01, (off_t) debug_trap, 0);
  DTman->set_trap_gate(0x02, (off_t) NMI_trap, 0);
  DTman->set_trap_gate(0x03, (off_t) & int3, 3);	/* прерывания 3-5 могут вызываться из задач */
  DTman->set_trap_gate(0x04, (off_t) & overflow, 3);
  DTman->set_trap_gate(0x05, (off_t) & BR, 3);
  DTman->set_trap_gate(0x06, (off_t) & invalid_operation_trap, 0);
  DTman->set_trap_gate(0x07, (off_t) & FPU_not_present_trap, 0);
  DTman->set_trap_gate(0x08, (off_t) & double_fault_trap, 0);
  DTman->set_trap_gate(0x09, (off_t) & reserved_trap, 0);
  DTman->set_trap_gate(0x0A, (off_t) & invalid_TSS_trap, 0);
  DTman->set_trap_gate(0x0B, (off_t) & segment_not_present_trap, 0);
  DTman->set_trap_gate(0x0D, (off_t) & general_protection_fault_trap, 0);
  DTman->set_trap_gate(0x0E, (off_t) & page_fault_trap, 0);
  DTman->set_trap_gate(0x0F, (off_t) & reserved_trap, 0);
  DTman->set_trap_gate(0x10, (off_t) & FPU_error_trap, 0);
  DTman->set_trap_gate(0x11, (off_t) & align_error_trap, 0);
  DTman->set_trap_gate(0x12, (off_t) & machine_depend_error_trap, 0);
  for (i = 0x13; i < 0x20; i++)
    DTman->set_trap_gate(i, (off_t) & reserved_trap, 0);

  for (i = 0x20; i < 0x100; i++)
    DTman->set_trap_gate(i, (off_t) & interrupt_hdl_not_present_trap, 0);

  DTman->set_intr_gate(0x20, (off_t) & timer_handler_wrapper);

  DTman->set_intr_gate(0x21, (off_t) & keyboard_handler_wrapper);

  DTman->set_intr_gate(0x26, (off_t) & floppy_handler_wrapper);

  DTman->set_trap_gate(0x30, (off_t) & trap_gate, 3);
  //  DTman->set_call_gate(BASE_TSK_SEL_N-1, (off_t)&call_gate, 3, 4);
}
