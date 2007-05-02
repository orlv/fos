/* 
	kernel/main/ints.S
	Copyright (C) 2004-2007 Oleg Fedorov 
*/

//.globl timer_handler_wrapper
.globl floppy_handler_wrapper
.globl keyboard_handler_wrapper
	
.text
.align 4
floppy_handler_wrapper:
    pusha
    push %ds
    push %es

    mov $0x10, %ax
    mov %ax,%ds
    mov %ax,%es

    call floppy_handler

    pop %es
    pop %ds
    popa
    iret

.align 4
keyboard_handler_wrapper:
    pusha
    push %ds
    push %es

    mov $0x10, %ax
    mov %ax,%ds
    mov %ax,%es

    call keyboard_handler
	
    pop %es
    pop %ds
    popa
    iret
