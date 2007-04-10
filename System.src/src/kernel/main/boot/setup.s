/*
	kernel/main/boot/setup.s
	Copyright (C) 2005-2006 Oleg Fedorov
*/

.globl _start
.globl __mbi
	
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
	mov $0x9FFFF, %esp

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

	/* настроим PIC (8259) */
	mov $0x11,%al
	out %al,$0x20
	out %al,$0xA0
	mov $0x20,%al	/* IRQ 0x00-0x07 теперь 0x20-0x27 */
	out %al,$0x21
	mov $0x28,%al	/* IRQ 0x08-0x0A теперь 0x28-0x2F */
	out %al,$0xA1
	mov $0x04,%al
	out %al,$0x21
	mov $0x02,%al
	out %al,$0xA1
	mov $0x01,%al
	out %al,$0x21
	mov $0x01,%al
	out %al,$0xA1

	/* запретим IRQ 0-7 */
	mov $0xff,%al
	out %al,$0x21

	/* запретим IRQ 8-15 */
	mov $0xff,%al
	out %al,$0xA1

	sti

	/* загрузим gdtr */
	lgdt __gdtr

	mov $0x0010, %ax
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%fs
	mov %ax,%gs
	mov %ax,%ss

	mov %ebx, __mbi
	//push %ebx        /* multiboot_info_t */
	//call get_memory_boot_info
	//add $4, %esp

	call init_memory
	call setup_idt   /* Установим обработчики прерываний */

	call init

halt:	hlt
	jmp halt

/* временная Глобальная Таблица Дескрипторов (GDT) */
.align 4
.word 0
__gdtr:
	.word 3*8-1      /* Лимит                       */
	.long tmp_gdt    /* Указатель на gdt            */

tmp_gdt:
	.long 0          /* Нулевой дескриптор          */
	.long 0
	.long 0x0000ffff /* (0x08) Kernel 0-4 Гб код    */
	.long 0x00cf9a00
	.long 0x0000ffff /* (0x10) Kernel 0-4 Гб данные */
	.long 0x00cf9200

.align 4
__mbi:
	.long 0
