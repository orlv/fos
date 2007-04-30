/*
  kernel/main/exception/traps.cpp
  Copyright (C) 2004-2007 Oleg Fedorov
*/

#include <traps.h>
#include <stdio.h>
#include <system.h>
#include <mm.h>
#include <hal.h>
#include <tasks.h>

/*
  Из файла kernel/exceprion/traps.s:
*/

asmlinkage void timer_handler_wrapper();
asmlinkage void floppy_handler_wrapper();
asmlinkage void keyboard_handler_wrapper();

asmlinkage void trap_gate();

#define EXCEPTION_HANDLER(func) extern "C" void func (unsigned int errcode); \
  asm(".globl " #func"\n"						\
      #func ": \n"							\
      "pusha \n"							\
      "push %ds \n"							\
      "push %es \n"							\
      "mov $0x10, %ax \n"     /* загрузим DS ядра */			\
      "mov %ax, %ds \n"							\
      "mov %ax, %es \n"							\
      "mov 48(%esp), %eax \n" /* сохраним errorcode */			\
      "push %eax \n"							\
      "mov 48(%esp), %eax \n" /* сохраним eip */			\
      "push %eax \n"							\
      "xor %eax, %eax \n"						\
      "mov 48(%esp), %ax \n"  /* сохраним cs */				\
      "push %eax \n"							\
      "call _" #func " \n"						\
      "add $10, %esp \n"						\
      "pop %es \n"							\
      "pop %ds \n"							\
      "popa \n"								\
      "iret");								\
  extern "C" void _ ## func(unsigned int cs, unsigned int address, unsigned int errorcode)

void exception(string str, unsigned int cs,  unsigned int address, unsigned int errorcode)
{
  hal->cli();

  printk("\n--------------------------------------------------------------------------------");
  printk("Exception: %s \n", str);
  printk("\nAt addr: 0x%02X:0x%08lX\n", cs, address);
  u16_t id = curPID();
  printk("ID: %d \n", id);
  if (hal->ProcMan->CurrentProcess)
    printk("PID: %d \n", hal->ProcMan->CurrentProcess->pid);
  printk("Errorcode: 0x%X\n",errorcode);
  printk("System Halted!\n");
  printk("--------------------------------------------------------------------------------");
  hal->halt();
}

EXCEPTION_HANDLER(divide_error_exception)
{
  exception("[0x00] Divide Error", cs, address, errorcode);
}

EXCEPTION_HANDLER(debug_exception)
{
  exception("[0x01] Debug", cs, address, errorcode);
}

EXCEPTION_HANDLER(NMI_exception)
{
  exception("[0x02] NMI", cs, address, errorcode);
}

EXCEPTION_HANDLER(int3_exception)
{
  exception("int3", cs, address, errorcode);
}

EXCEPTION_HANDLER(overflow_exception)
{
  exception("[0x04] overflow", cs, address, errorcode);
}

EXCEPTION_HANDLER(bound_exception)
{
  exception("[0x05] bound", cs, address, errorcode);
}

EXCEPTION_HANDLER(invalid_operation_exception)
{
  exception("[0x06] invalid operation", cs, address, errorcode);
}

EXCEPTION_HANDLER(FPU_not_present_exception)
{
  exception("[0x07] FPU not present", cs, address, errorcode);
}

EXCEPTION_HANDLER(double_fault_exception)
{
  exception("[0x08] double fault", cs, address, errorcode);
}

EXCEPTION_HANDLER(reserved_exception)
{
  exception("reserved interrupt", cs, address, errorcode);
}

EXCEPTION_HANDLER(invalid_TSS_exception)
{
  exception("[0x0A] invalid TSS", cs, address, errorcode);
}

EXCEPTION_HANDLER(segment_not_present_exception)
{
  exception("[0x0B] segment not present", cs, address, errorcode);
}

EXCEPTION_HANDLER(stack_fault_exception)
{
  exception("[0x0C] stack fault", cs, address, errorcode);
}

EXCEPTION_HANDLER(general_protection_fault_exception)
{
  exception("[0x0D] general protection fault", cs, address, errorcode);
}

EXCEPTION_HANDLER(page_fault_exception)
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

  exception("[0x0E] page fault", cs, address, errorcode);
}

EXCEPTION_HANDLER(FPU_error_exception)
{
  exception("[0x10] FPU error", cs, address, errorcode);
}

EXCEPTION_HANDLER(align_error_exception)
{
  exception("[0x11] align error", cs, address, errorcode);
}

EXCEPTION_HANDLER(machine_depend_error_exception)
{
  exception("[0x12] machine depend error", cs, address, errorcode);
}

EXCEPTION_HANDLER(interrupt_hdl_not_present_exception)
{
  exception("interrupt_hdl_not_present", cs, address, errorcode);
}

void setup_idt()
{
  u16_t i;

  hal->idt->set_trap_gate(0x00, (off_t) & divide_error_exception, 0);
  hal->idt->set_trap_gate(0x01, (off_t) & debug_exception, 0);
  hal->idt->set_trap_gate(0x02, (off_t) & NMI_exception, 0);
  hal->idt->set_trap_gate(0x03, (off_t) & int3_exception, 3);	/* прерывания 3-5 могут вызываться из задач */
  hal->idt->set_trap_gate(0x04, (off_t) & overflow_exception, 3);
  hal->idt->set_trap_gate(0x05, (off_t) & bound_exception, 3);
  hal->idt->set_trap_gate(0x06, (off_t) & invalid_operation_exception, 0);
  hal->idt->set_trap_gate(0x07, (off_t) & FPU_not_present_exception, 0);
  hal->idt->set_trap_gate(0x08, (off_t) & double_fault_exception, 0);
  hal->idt->set_trap_gate(0x09, (off_t) & reserved_exception, 0);
  hal->idt->set_trap_gate(0x0A, (off_t) & invalid_TSS_exception, 0);
  hal->idt->set_trap_gate(0x0B, (off_t) & segment_not_present_exception, 0);
  hal->idt->set_trap_gate(0x0D, (off_t) & general_protection_fault_exception, 0);
  hal->idt->set_trap_gate(0x0E, (off_t) & page_fault_exception, 0);
  hal->idt->set_trap_gate(0x0F, (off_t) & reserved_exception, 0);
  hal->idt->set_trap_gate(0x10, (off_t) & FPU_error_exception, 0);
  hal->idt->set_trap_gate(0x11, (off_t) & align_error_exception, 0);
  hal->idt->set_trap_gate(0x12, (off_t) & machine_depend_error_exception, 0);
  for (i = 0x13; i < 0x20; i++)
    hal->idt->set_trap_gate(i, (off_t) & reserved_exception, 0);

  for (i = 0x20; i < 0x100; i++)
    hal->idt->set_trap_gate(i, (off_t) & interrupt_hdl_not_present_exception, 0);

  hal->idt->set_intr_gate(0x20, (off_t) & timer_handler_wrapper);
  hal->idt->set_intr_gate(0x21, (off_t) & keyboard_handler_wrapper);
  hal->idt->set_intr_gate(0x26, (off_t) & floppy_handler_wrapper);

  hal->idt->set_trap_gate(0x30, (off_t) & trap_gate, 3);
  //  hal->idt->set_call_gate(BASE_TSK_SEL_N-1, (off_t)&call_gate, 3, 4);
}
