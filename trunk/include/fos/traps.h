/*
    fos/traps.h
    Copyright (C) 2005-2007 Oleg Fedorov
*/

#ifndef _FOS_TRAPS_H
#define _FOS_TRAPS_H

#include <types.h>

void setup_idt();

#define EXCEPTION_HANDLER(func) extern "C" void func (unsigned int errcode); \
  asm(".globl " #func"\n"						\
      #func ": \n"							\
      "pusha \n"							\
      "push %ds \n"							\
      "push %es \n"							\
      "mov $0x10, %ax \n"     /* загрузим DS ядра */			\
      "mov %ax, %ds \n"							\
      "mov %ax, %es \n"							\
      "mov 40(%esp), %eax \n" /* сохраним errorcode */			\
      "push %eax \n"							\
      "mov 48(%esp), %eax \n" /* сохраним eip */			\
      "push %eax \n"							\
      "xor %eax, %eax \n"						\
      "mov 56(%esp), %ax \n"  /* сохраним cs */				\
      "push %eax \n"							\
      "call _" #func " \n"						\
      "add $12, %esp \n"						\
      "pop %es \n"							\
      "pop %ds \n"							\
      "popa \n"								\
      "iret");								\
  asmlinkage void _ ## func(unsigned int cs, unsigned int address, unsigned int errorcode)

#define IRQ_HANDLER(func) extern "C" void func (unsigned int errcode);	\
  asm(".globl " #func"\n"						\
      #func ": \n"							\
      "pusha \n"							\
      "push %ds \n"							\
      "push %es \n"							\
      "mov $0x10, %ax \n"     /* загрузим DS ядра */			\
      "mov %ax, %ds \n"							\
      "mov %ax, %es \n"							\
      "mov 48(%esp), %eax \n" /* сохраним eip */			\
      "push %eax \n"							\
      "xor %eax, %eax \n"						\
      "mov 56(%esp), %ax \n"  /* сохраним cs */				\
      "push %eax \n"							\
      "call _" #func " \n"						\
      "add $8, %esp \n"							\
      "pop %es \n"							\
      "pop %ds \n"							\
      "popa \n"								\
      "iret");								\
  asmlinkage void _ ## func(unsigned int cs, unsigned int address)

#endif
