/*
	main.s
	Copyright (C) 2007 Oleg Fedorov
*/
.code32
.globl _start
	
.text
_start:
	pushl %ebp

//	movw %ax, reg_ax
//	movw %bx, reg_bx
	
//	movw %cx, reg_di
//	movw %dx, reg_es
	
	call prot_to_real

	.code16

//	movw reg_ax, %ax

//	movw reg_bx, %bx

//	movw reg_di, %dx
//	movw %dx, %di

//	movw reg_es, %dx
//	movw %dx, %es


	movw $0x4000, %ax
	movw %ax, %es
	xor %di, %di
	
	movw $0x4f01, %ax
	movw %bx, %cx

	int $0x10


	/* В %bx должен находится номер режима */
	movw $0x4f02, %ax
	//movw $0x4111, %bx
	
	int	$0x10
	
	movw %ax, reg_ax

//	movw $3, %ax
//	int $0x10
//	movw %ax, %dx

	data32 call real_to_prot

.code32
	xor %eax, %eax
	movw reg_ax, %ax

	popl %ebp		
	ret

/* ------------------------------- */

real_to_prot:
	.code16
	cli
	
	/* turn on protected mode */
	movl	%cr0, %eax
	//orl	$GRUB_MEMORY_MACHINE_CR0_PE_ON, %eax
	orl	$0x1, %eax
	movl	%eax, %cr0

	/* jump to relocation, flush prefetch queue, and reload %cs */
	//ljmp	$GRUB_MEMORY_MACHINE_PROT_MODE_CSEG, $protcseg
	ljmp $0x08, $protcseg 

	.code32
protcseg:
	/* reload other segment registers */
//	movw	$GRUB_MEMORY_MACHINE_PROT_MODE_DSEG, %ax
	movw	$0x10, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	movw	%ax, %ss

	/* put the return address in a known safe location */
	movl	(%esp), %eax
//	movl	%eax, GRUB_MEMORY_MACHINE_REAL_STACK
	movl	%eax, (0x1efc) /*(0x2000 - 0x104)*/


	/* get protected mode stack */
	movl	__pm_stack, %eax
	movl	%eax, %esp
	movl	%eax, %ebp

	/* get return address onto the right stack */
//	movl	GRUB_MEMORY_MACHINE_REAL_STACK, %eax
	movl	(0x1efc), %eax
	movl	%eax, (%esp)

	/* zero %eax */
	xorl	%eax, %eax

	lidt	__idt_pm
	sti
	/* return on the old (or initialized) stack! */
	ret
		
prot_to_real:
	sidt	__idt_pm  /* сохраним IDT от защищённого режима */
	lidt	__idt_real /* установим IDT для реального режима */

	/* save the protected mode stack */
	movl	%esp, %eax
	movl	%eax, __pm_stack
	
	/* get the return address */
	movl	(%esp), %eax
//	movl	%eax, GRUB_MEMORY_MACHINE_REAL_STACK
	movl	%eax, (0x2000 - 0x104)

	/* set up new stack */
//	movl	$GRUB_MEMORY_MACHINE_REAL_STACK, %eax
	movl	$(0x2000 - 0x104), %eax
	movl	%eax, %esp
	movl	%eax, %ebp

	/* set up segment limits */
//	movw	$GRUB_MEMORY_MACHINE_PSEUDO_REAL_DSEG, %ax
	movw	$0x30, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	movw	%ax, %ss

	/* this might be an extra step */
	/* jump to a 16 bit segment */
//	ljmp	$GRUB_MEMORY_MACHINE_PSEUDO_REAL_CSEG, $tmpcseg
	ljmp	$0x28, $tmpcseg   /* 0x18 */

tmpcseg:
	.code16

	/* clear the PE bit of CR0 */
	movl	%cr0, %eax
//	andl 	$(~GRUB_MEMORY_MACHINE_CR0_PE_ON), %eax
	andl 	$(~0x1), %eax
	movl	%eax, %cr0

	/* flush prefetch queue, reload %cs */
	data32 ljmp $0, $realcseg

realcseg:
	/* we are in real mode now
	 * set up the real mode segment registers : DS, SS, ES
	 */
	/* zero %eax */
	xorl	%eax, %eax

	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	movw	%ax, %ss

	/* restore interrupts */
//	sti

	/* return on new stack! */
	DATA32	ret

/* ------------------------------------------------------------ */
.code32
.align 4
__pm_stack:
	.long 0

__idt_real:
	.word 0x3ff, 0, 0

__idt_pm:
	.word 0
	.long 0

reg_ax:
	.word 0
/*
reg_bx:	
	.word 0
reg_cx:
	.word 0
reg_dx:
	.word 0
*/
/* ------------------------------------------------------------ */
