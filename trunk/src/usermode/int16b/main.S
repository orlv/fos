/*
  main.S
  Copyright (C) 2007 Oleg Fedorov

  Проверяет возможность установки видеорежима,
  в случае успеха устанавливает режим
  и возвращает указатель на буфер с информацией о нем

  При написании этого кода я пользовался исходными текстами GRUB
*/

#define STACK_REALMODE	0x0000fffc

.globl _start
.text
.code32
_start:
	pushal

	movl	%eax, %ecx
	movl	%eax, %ebx

	movl    $vbe_mode_info, %edx	/* вычисляем сегмент:смещение (dx:di) для vbe_mode_info */
	movw	%dx, %di
	xorw	%dx, %dx
	shrl	$4, %edx

	sidt	idt_pmode		/* сохраним IDT защищённого режима	*/
	lidt	idt_rmode		/* установим IDT для реального режима	*/
	sgdt	gdtdesc_pmode		/* сохраняем GDT защищенного режима	*/

	movl	%esp, %eax		/* сохраним стек защищенного режима	*/
	movl	%eax, stack_pmode

	lgdt	gdtdesc_tmp		/* загружаем временный GDT (содержит сегменты 16-битного режима) */
	ljmp	$0x18, $realmode_seg	/* Long Jump в 16-битный сегмент 	*/

.code16
realmode_seg:

	movl	%cr0, %eax		/* отключаем страничную адресацию и защищенный режим */
	andl 	$~0x80000001, %eax
	movl	%eax, %cr0

	jmp	$0, $1f			/* прыжок в сегмент 0 			*/
1:
	xorw	%ax, %ax		/* загружаем сегментные регистры 	*/
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	movw	%ax, %ss

	movl	$STACK_REALMODE, %eax
	movl	%eax, %esp
	movl	%eax, %ebp

/*
	чтение информации о видеорежиме
		ax = 0x4f01
		cx = номер режима
		es:di = адрес буфера для сохранения информакии (256 байт)
	возвращает:
		ax = 0x4f в случае успеха
*/
	movw	%dx, %es
	movw	$0x4f01, %ax
	int	$0x10

	cmp	$0x4f, %ax		/* режим поддерживается?		*/
	jne	exit_to_pm		/* нет, возврат в PM 			*/
	
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

exit_to_pm:
	movw	%ax, %bx		/* сохраним код завершения		*/

	mov %cr0, %eax			/* включим защищенный режим и страничную адресацию */
	or $0x80000001, %eax
	mov %eax, %cr0

	ljmpl $0x08, $pmode_segment

.code32
pmode_segment:
	lgdt	gdtdesc_pmode		/* восстаовим таблицу дескрипторов	*/
	movw	$0x10, %ax		/* загрузим сегментные регистры		*/
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	movw	%ax, %ss

	lidt	idt_pmode		/* восстановим таблицу прерываний	*/
	movl	stack_pmode, %esp	/* восстановим стек защищенного режима	*/

	xorl	%eax, %eax

	cmp	$0x4f, %bx
	jne	error_done

	/* если режим установлен успешно - возвращаем
	   указатель на буфер с информацией об режиме */
	popal
	movl	$vbe_mode_info, %eax
	ret

error_done:
	popal
	ret

.code32
.p2align 2
stack_pmode:
	.long 0

idt_rmode:
	.word 0x3ff
	.long 0

idt_pmode:
	.word 0
	.long 0

.p2align 2
gdt_tmp:
        .word   0, 0
        .byte   0, 0, 0, 0

        .word   0xFFFF, 0		        /* сегмент кода			*/
        .byte   0, 0x9A, 0xCF, 0

        .word   0xFFFF, 0			/* сегмент данных		*/
        .byte   0, 0x92, 0xCF, 0

        .word   0xFFFF, 0			/* 16-битный сегмент кода	*/
        .byte   0, 0x9E, 0, 0

        .word   0xFFFF, 0			/* 16-битный сегмент данных	*/
        .byte   0, 0x92, 0, 0


gdtdesc_tmp:					/* временный дескриптор GDT */
        .word   0x27				/* лимит */
        .long   gdt_tmp				/* адрес */

gdtdesc_pmode:
        .word   0				/* лимит */
        .long   0				/* адрес */


vbe_mode_info:					/* буфер под информацию о видеорежиме */
	.fill 256, 1, 0
