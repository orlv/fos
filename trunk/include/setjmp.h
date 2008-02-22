/*
 * from dietlibc
 */

#ifndef SETJMP_H
#define SETJMP_H

#ifndef __ASSEMBLER__
typedef long __jmp_buf[6];
#endif

#define JB_BX	0
#define JB_SI	1
#define JB_DI	2
#define JB_BP	3
#define JB_SP	4
#define JB_PC	5
#define JB_SIZE 24

#ifndef __ASSEMBLER__

typedef struct {
  unsigned long sig[(64 / sizeof(long)) >> 3];
} sigset_t;

typedef struct __jmp_buf_tag {	/* C++ doesn't like tagless structs.  */
  /* NOTE: The machine-dependent definitions of `__sigsetjmp'
   * assume that a `jmp_buf' begins with a `__jmp_buf'.
   * Do not move this member or add others before it.      */
  __jmp_buf __jmpbuf;		/* Calling environment.    */
  int __mask_was_saved;		/* Saved the signal mask?  */
  sigset_t __saved_mask;	/* Saved signal mask.      */
} jmp_buf[1];

typedef jmp_buf sigjmp_buf;

void longjmp(sigjmp_buf env,int val);
int setjmp(jmp_buf env);
int sigsetjmp(sigjmp_buf env, int savesigs);

#endif /* __ASSEMBLER__ */

#endif
