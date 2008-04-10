.globl realmode_code
.text
.code16
realmode_code:
	cli
	movw	%cs, %ax
	movw	%ax, %ds
	movw	%ax, %ss
	movw	%ax, %es
	movw	$0x9000, %ax
	movw	%ax, %sp
	sti
	hlt
	xorw	%bx, %bx
	movw	$0x0E01, %ax
	int	$0x10
	jmp	realmode_code

