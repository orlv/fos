/*
	kernel/main/boot/setup.s
	Copyright (C) 2005-2007 Oleg Fedorov
*/

.globl _start
.globl __mbi
//.globl vesa_init
	
.text
_start:
	jmp multiboot_entry

.align 4
/* Заголовок Multiboot (используется загрузчиком GRUB) */
	multiboot_header:
	/* "магическое число" (MULTIBOOT_HEADER_MAGIC) */
	.long 0x1BADB002 
	/* флаги (MULTIBOOT_HEADER_FLAGS) */
	.long 0x00000003
	/* checksum */
	.long -(0x1BADB002 + 0x00000003)

multiboot_entry:
	
	/* Инициализируем стек ядра */
	mov $0x9FB00, %esp

	/*
	  Сбросим флаг DF в 0, при последующих строковых
	  операциях регистры edi и esi будут уменьшаться.
	*/
	cld

	/* сбросим EFLAGS.  */
	pushl $0
	popf

	cli

	/* отключим немаскируемые прерывания (NMI) */
	in $0x70,%al
	or $0x80,%al
	out %al,$0x70
	in $0x71,%al

	mov %ebx, __mbi

	call init

halt:	hlt
	jmp halt

.align 4
__mbi:
	.long 0
