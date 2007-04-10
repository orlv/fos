/*
        kernel/include/tss.h
        Copyright (C) 2005 Oleg Fedorov
*/

#ifndef __TSS_H
#define __TSS_H

#include <types.h>

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
  u16_t io_map_addr;
  u32_t	IOPB;
  /* u8_t io_map[8192]; */
}__attribute__((packed));

#endif
