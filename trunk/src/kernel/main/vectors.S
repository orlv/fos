/*
 * kernel/main/vectors.S
 * Copyright (C) 2008 Sergey Gridassov
 * TODO: переместить в arch вместе в ints.cpp и idt.cpp
 */

vector=0
.globl interrupt_vectors_start, interrupt_vectors_end

interrupt_vectors_start:
.rept 221
      pusha
      push %ds
      push %es
      mov $0x10, %ax    /* загрузим DS ядра */
      mov %ax, %ds
      mov %ax, %es
      mov 48(%esp), %eax /* сохраним eip */
      push %eax	
      xor %eax, %eax	
      mov 56(%esp), %ax /* сохраним cs */
      push %eax
      push $vector
      call common_interrupt
      add $12, %esp
      pop %es
      pop %ds	
      popa
      iret
.align 16
vector=vector+1
.endr
interrupt_vectors_end:
