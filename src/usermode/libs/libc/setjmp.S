/*
 * from dietlibc
 */
#include <setjmp.h>

.text
.globl setjmp
.type setjmp,@function
setjmp:
	movl	0(%esp), %ecx	/* Save PC we are returning to now */
	leal	4(%esp), %edx	/* Save SP as it will be after we return */
	movl	(%edx), %eax

	pushl	$0
	pushl	%eax
	call	sigsetjmp_internal
	add	$8,%esp
	ret


.globl sigsetjmp
.type sigsetjmp,@function
sigsetjmp:
	movl	0(%esp), %ecx	/* Save PC we are returning to now.  */
	leal	4(%esp), %edx	/* Save SP as it will be after we return.  */
	movl	(%edx), %eax

sigsetjmp_internal:
	/* Save registers.  */
	movl %ebx, (JB_BX*4)(%eax)
	movl %esi, (JB_SI*4)(%eax)
	movl %edi, (JB_DI*4)(%eax)
	movl %ebp, (JB_BP*4)(%eax)
	movl %edx, (JB_SP*4)(%eax)
	movl %ecx, (JB_PC*4)(%eax)

	/* Make a tail call to __sigjmp_save; it takes the same args.  */
	jmp __sigjmp_save

