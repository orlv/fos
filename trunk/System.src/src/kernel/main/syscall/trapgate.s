/*
    kernel/main/syscall/trapgate.s
    Copyright (C) 2004-2006 Oleg Fedorov
*/

.globl trap_gate

.align 4
trap_gate:
	push %ebp
	pushal

	mov  %esp, %ebp

	pushl %ecx
	pushl %ebx

	mov $0x10, %ax
	mov %ax,%ds
	mov %ax,%es

	call sys_call

	add $8, %esp

	mov $0x23, %ax
	mov %ax, %ds
	mov %ax, %es

	popal
	pop %ebp

	iret
	