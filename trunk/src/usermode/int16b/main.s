/*
	main.s
	Copyright (C) 2007 Oleg Fedorov
*/
.code32
.globl _start
	
.text
_start:
	pushl %ebp
	pushl %edi
	pushl %es
//	movl	%esp, %ebp

	/* вычисляем сегмент:смещение для vbe_mode_info_block */
	movl    $vbeinfo, %eax
	movl	%eax, %edi
	andl	$0x0000000f, %edi
	shrl	$4, %eax
	movl	%eax, %ebx

	call	prot_to_real

	.code16

	/*
	чтение информации о режиме
		ax = 0x4f01
		cx = код интересующего режима
		es:di = адрес буфера для сохранения информакии (256 байт)
	возвращает:
		ax = 0x4f в случае успеха
	*/
	movw	%bx, %es
	movw	$0x4f01, %ax
	int	$0x10

	cmp	$0x4f, %ax	/* режим поддерживается? */
	jne	exit_to_pm	/* нет, возврат в PM */
	
	/*
	установка видеорежима
		ax = 0x4f02
		bx = номер режима
	возвращает:
		ax = 0x4f в случае успеха
	*/
	movw	$0x4f02, %ax
	movw	%cx, %bx
	int	$0x10
	mov	%ax, %bx
exit_to_pm:
	data32 call real_to_prot

.code32
	xor %eax, %eax
	mov %bx, %ax
	movl $vbeinfo, %ebx
	popl %es
	popl %edi
	popl %ebp		
	ret

/* ------------------------------- */

real_to_prot:
	.code16
	cli
	
	/* turn on protected mode */
	movl	reg_cr0, %eax
	movl	%eax, %cr0
	/* jump to relocation, flush prefetch queue, and reload %cs */
	ljmp $0x08, $protcseg 

	.code32
protcseg:
	lgdt	gdtdesc_pm
	/* reload other segment registers */
	movw	$0x10, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	movw	%ax, %ss

	/* put the return address in a known safe location */
	movl	(%esp), %eax
	movl	%eax, 0x8000


	/* get protected mode stack */
	movl	__pm_stack, %eax
	movl	%eax, %esp
	movl	%eax, %ebp

	/* get return address onto the right stack */
	movl	0x8000, %eax
	movl	%eax, (%esp)

	/* zero %eax */
	xorl	%eax, %eax

	lidt	__idt_pm
//	sti
	/* return on the old (or initialized) stack! */
	ret
		
prot_to_real:
	sidt	__idt_pm  /* сохраним IDT от защищённого режима */
	lidt	__idt_real /* установим IDT для реального режима */
	sgdt	gdtdesc_pm
	lgdt	gdtdesc

	/* save the protected mode stack */
	movl	%esp, %eax
	movl	%eax, __pm_stack
	
	/* get the return address */
	movl	(%esp), %eax
	movl	%eax, 0x8000

	/* set up new stack */
	movl	$0x8000, %eax
	movl	%eax, %esp
	movl	%eax, %ebp

	/* set up segment limits */
	movw	$0x20, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	movw	%ax, %ss

	/* this might be an extra step */
	/* jump to a 16 bit segment */
	ljmp	$0x18, $tmpcseg

tmpcseg:
	.code16

	/* clear the PE bit of CR0 */
	movl	%cr0, %eax
	movl	%eax, reg_cr0
	andl 	$0x7ffffffe, %eax
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
	//sti
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

reg_cr0:
	.long 0
/*
reg_ax:
	.word 0

reg_bx:	
	.word 0
reg_cx:
	.word 0
reg_dx:
	.word 0
*/

	.p2align        2       /* force 4-byte alignment */
gdt:
        .word   0, 0
        .byte   0, 0, 0, 0

        /* code segment */
        .word   0xFFFF, 0
        .byte   0, 0x9A, 0xCF, 0

        /* data segment */
        .word   0xFFFF, 0
        .byte   0, 0x92, 0xCF, 0

        /* 16 bit real mode CS */
        .word   0xFFFF, 0
        .byte   0, 0x9E, 0, 0

        /* 16 bit real mode DS */
        .word   0xFFFF, 0
        .byte   0, 0x92, 0, 0


/* this is the GDT descriptor */
gdtdesc:
        .word   0x27                    /* limit */
        .long   gdt                     /* addr */

gdtdesc_pm:
        .word   0                    /* limit */
        .long   0                     /* addr */


vbeinfo:
	.fill 256, 1, 0

/* ------------------------------------------------------------ */
