/*
    fos/arch/i386/include/tss.h
    Copyright (C) 2005 Oleg Fedorov
*/

#ifndef _FOS_TSS_H
#define _FOS_TSS_H

#include <types.h>

/*
 * EFLAGS bits
 */
#define X86_EFLAGS      0x00000002
#define X86_EFLAGS_CF   0x00000001 /* Carry Flag */
#define X86_EFLAGS_PF   0x00000004 /* Parity Flag */
#define X86_EFLAGS_AF   0x00000010 /* Auxillary carry Flag */
#define X86_EFLAGS_ZF   0x00000040 /* Zero Flag */
#define X86_EFLAGS_SF   0x00000080 /* Sign Flag */
#define X86_EFLAGS_TF   0x00000100 /* Trap Flag */
#define X86_EFLAGS_IF   0x00000200 /* Interrupt Flag */
#define X86_EFLAGS_DF   0x00000400 /* Direction Flag */
#define X86_EFLAGS_OF   0x00000800 /* Overflow Flag */
#define X86_EFLAGS_IOPL 0x00003000 /* IOPL mask */
#define X86_EFLAGS_NT   0x00004000 /* Nested Task */
#define X86_EFLAGS_RF   0x00010000 /* Resume Flag */
#define X86_EFLAGS_VM   0x00020000 /* Virtual Mode */
#define X86_EFLAGS_AC   0x00040000 /* Alignment Check */
#define X86_EFLAGS_VIF  0x00080000 /* Virtual Interrupt Flag */
#define X86_EFLAGS_VIP  0x00100000 /* Virtual Interrupt Pending */
#define X86_EFLAGS_ID   0x00200000 /* CPUID detection flag */

/*
 * Intel CPU features in CR4
 */
#define X86_CR4_VME             0x0001  /* enable vm86 extensions */
#define X86_CR4_PVI             0x0002  /* virtual interrupts flag enable */
#define X86_CR4_TSD             0x0004  /* disable time stamp at ipl 3 */
#define X86_CR4_DE              0x0008  /* enable debugging extensions */
#define X86_CR4_PSE             0x0010  /* enable page size extensions */
#define X86_CR4_PAE             0x0020  /* enable physical address extensions */
#define X86_CR4_MCE             0x0040  /* Machine check enable */
#define X86_CR4_PGE             0x0080  /* enable global pages */
#define X86_CR4_PCE             0x0100  /* enable performance counters at ipl 3 */
#define X86_CR4_OSFXSR          0x0200  /* enable fast FPU save and restore */
#define X86_CR4_OSXMMEXCPT      0x0400  /* enable unmasked SSE exceptions */

#define IO_BITMAP_BITS  65536
#define IO_BITMAP_BYTES (IO_BITMAP_BITS/8)
#define IO_BITMAP_LONGS (IO_BITMAP_BYTES/sizeof(long))

struct TSS
{
  u32_t link;
  u32_t esp0;
  u32_t ss0;
  u32_t esp1;
  u32_t ss1;
  u32_t esp2;
  u32_t ss2;
  u32_t cr3;
  u32_t eip;
  u32_t eflags;
  u32_t eax;
  u32_t ecx;
  u32_t edx;
  u32_t ebx;
  u32_t esp;
  u32_t ebp;
  u32_t esi;
  u32_t edi;
  u32_t es;
  u32_t cs;
  u32_t ss;
  u32_t ds;
  u32_t fs;
  u32_t gs;
  u32_t ldtr;
  u16_t	trace;
  u16_t io_bitmap_base;
  /*
   * The extra 1 is there because the CPU will access an
   * additional byte beyond the end of the IO permission
   * bitmap. The extra byte must be all 1 bits, and must
   * be within the limit.
   */
#if 0
  u32_t io_bitmap[IO_BITMAP_LONGS + 1];
#endif
}__attribute__((packed));

#endif
