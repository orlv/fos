/*
	startup.s
	Copyright (C) 2006 Oleg Fedorov
*/

.globl _start
	
.text
_start:
	call init

		
halt:	int $3
	jmp halt
