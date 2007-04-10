/* 
	kernel/main/ints.S
	Copyright (C) 2004-2006 Oleg Fedorov 
*/

/* Исключения */
.globl divide_error_trap
.globl debug_trap
.globl NMI_trap
.globl int3
.globl overflow
.globl BR
.globl invalid_operation_trap
.globl FPU_not_present_trap
.globl double_fault_trap
.globl reserved_trap
.globl invalid_TSS_trap
.globl segment_not_present_trap
.globl stack_fault_trap
.globl general_protection_fault_trap
.globl page_fault_trap
.globl FPU_error_trap
.globl align_error_trap
.globl machine_depend_error_trap

.globl interrupt_hdl_not_present_trap

.globl timer_handler_wrapper
.globl floppy_handler_wrapper
.globl keyboard_handler_wrapper
	
.text
.align 4
timer_handler_wrapper:
    pusha
    push %ds
    push %es

    mov $0x10, %ax
    mov %ax,%ds
    mov %ax,%es

    movw 40(%esp), %ax /* сохраним cs */
    push %ax
    call timer_handler
    add $2, %esp

    pop %es
    pop %ds
    popa
    iret

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


.align 4
divide_error_trap:
	movl $0x10,%edx
	mov %dx,%ds
	mov %dx,%es
	call divide_error_handler
	iret

.align 4
debug_trap:
	movl $0x10,%edx
	mov %dx,%ds
	mov %dx,%es
	mov $0x20, %ax
	mov $0x20, %dx
	outb %al, %dx
	call debug_handler
	iret	

.align 4
NMI_trap:
	movl $0x10,%edx
	mov %dx,%ds
	mov %dx,%es
	call NMI_handler
	iret

.align 4
int3:
	movl $0x10,%edx
	mov %dx,%ds
	mov %dx,%es
	call int3_handler
	iret

.align 4
overflow:
	movl $0x10,%edx
	mov %dx,%ds
	mov %dx,%es
	call overflow_handler
	iret

.align 4
BR:
	movl $0x10,%edx
	mov %dx,%ds
	mov %dx,%es
	call bound_handler
	iret

.align 4
invalid_operation_trap:
	movl $0x10,%edx
	mov %dx,%ds
	mov %dx,%es
	call invalid_operation_handler
	iret

.align 4
FPU_not_present_trap:
	movl $0x10,%edx
	mov %dx,%ds
	mov %dx,%es
	call FPU_not_present_handler
	iret

.align 4
double_fault_trap:
	movl $0x10,%edx
	mov %dx,%ds
	mov %dx,%es
	call double_fault_handler
	iret

.align 4
reserved_trap:
	movl $0x10,%edx
	mov %dx,%ds
	mov %dx,%es
	call reserved_handler
	iret

.align 4
invalid_TSS_trap:
	movl $0x10,%edx
	mov %dx,%ds
	mov %dx,%es
	call invalid_TSS_handler
	iret

.align 4
segment_not_present_trap:
	movl $0x10,%edx
	mov %dx,%ds
	mov %dx,%es
	call segment_not_present_handler
	iret

.align 4
stack_fault_trap:
	movl $0x10,%edx
	mov %dx,%ds
	mov %dx,%es
	call stack_fault_handler
	iret

.align 4
general_protection_fault_trap:
	mov $0x10,%edx
	mov %dx,%ds
	mov %dx,%es
	call general_protection_fault_handler
	iret

.align 4
page_fault_trap:
	movl $0x10,%edx
	mov %dx,%ds
	mov %dx,%es
	call page_fault_handler
	iret

.align 4
FPU_error_trap:
	movl $0x10,%edx
	mov %dx,%ds
	mov %dx,%es
	call FPU_error_handler
	iret

.align 4
align_error_trap:
	movl $0x10,%edx
	mov %dx,%ds
	mov %dx,%es
	call align_error_handler
	iret

.align 4	
machine_depend_error_trap:
	movl $0x10,%edx
	mov %dx,%ds
	mov %dx,%es
	call machine_depend_error_handler
	iret

.align 4	
interrupt_hdl_not_present_trap:
	movl $0x10,%edx
	mov %dx,%ds
	mov %dx,%es
	call interrupt_hdl_not_present_handler
	iret
