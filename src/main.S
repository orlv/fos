/*
	main.S
	Copyright (C) 2007 Oleg Fedorov
*/

.globl _start
.text
.code32
_start:
	//pushal
	pushl	%ebp
	//movl	%esp, %ebp
	pushl	%edi
	pushl	%ebx
	pushl	%ecx

	movl	%eax, %ecx
	movl	%eax, %ebx

	/* вычисляем сегмент:смещение для vbe_mode_info_block */
	movl    $vbe_mode_info, %edx
	movw	%dx, %di
	xorw	%dx, %dx
	shrl	$4, %edx

//	call	prot_to_real
/* ------------------------------------------------- */
//prot_to_real:
	sidt	idt_pmode	/* сохраним IDT защищённого режима    */
	lidt	idt_rmode	/* установим IDT для реального режима */
	sgdt	gdtdesc_pmode
	lgdt	gdtdesc_tmp

	/* save the protected mode stack */
	movl	%esp, %eax
	movl	%eax, stack_pmode
	/* this might be an extra step */
	/* jump to a 16 bit segment */
	ljmp	$0x18, $real_seg_1

.code16
real_seg_1:
	/* clear the PE bit of CR0 */
	movl	%cr0, %eax
	//movl	%eax, reg_cr0
	andl 	$~0x80000001, %eax
	movl	%eax, %cr0

	/* flush prefetch queue, reload %cs */
	jmp	$0, $real_seg_2

real_seg_2:
	/* we are in real mode now
	 * set up the real mode segment registers : DS, SS, ES
	 */
	xorw	%ax, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	movw	%ax, %ss

	movl %esp, %eax
	movl $0x0000fffc, %esp
	movl $0x0000fffc, %ebp
	pushl %eax

/* ------------------------------------------- */
	/*
	чтение информации о режиме
		ax = 0x4f01
		cx = код интересующего режима
		es:di = адрес буфера для сохранения информакии (256 байт)
	возвращает:
		ax = 0x4f в случае успеха
	*/
	//pushw	%es
	//pushf
	//pusha
	movw	%dx, %es
	movw	$0x4f01, %ax
	int	$0x10
	//popa
	//popf
	//popw	%es
	
	cmp	$0x4f, %ax	/* режим поддерживается? */
	jne	exit_to_pm	/* нет, возврат в PM */
	
	/*
	установка видеорежима
		ax = 0x4f02
		bx = номер режима
	возвращает:
		ax = 0x4f в случае успеха
	*/
	//pusha
	movw	$0x4f02, %ax
	movw	%cx, %bx
	int	$0x10
	//popa

exit_to_pm:
	movw	%ax, %bx	/* сохраним код завершения */	

	mov %cr0, %eax		/* включим защищенный режим и страничную адресацию */
	or $0x80000001, %eax
	mov %eax, %cr0

	/* jump to relocation, flush prefetch queue, and reload %cs */
	ljmpl $0x08, $pmode_segment

.code32
pmode_segment:
	lgdt	gdtdesc_pmode		/* восстаовим таблицу дескрипторов */

	movw	$0x10, %ax		/* загрузим сегментные регистры */
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	movw	%ax, %ss

	lidt	idt_pmode		/* восстановим таблицу прерываний */
	movl	stack_pmode, %esp	/* восстановим стек защищенного режима */

	xorl	%eax, %eax
	cmp	$0x4f, %bx
	jne	done
	/*
		если режим установлен успешно - возвращаем
		указатель на буфер с информацией об режиме
	*/
	movl	$vbe_mode_info, %eax

done:

	popl	%ecx
	popl	%ebx
	popl	%edi
	popl	%ebp
	ret

.code32
.p2align        2
stack_pmode:
	.long 0

idt_rmode:
	.word 0x3ff
	.long 0

idt_pmode:
	.word 0
	.long 0

	/*
reg_cr0:
	.long 0
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
gdtdesc_tmp:
        .word   0x27		/* лимит */
        .long   gdt		/* адрес */

gdtdesc_pmode:
        .word   0		/* лимит */
        .long   0		/* адрес */


/* буфер под информацию о видеорежиме */
vbe_mode_info:
	.fill 256, 1, 0
