/*
    fos/gdt.h
    Copyright (C) 2006-2007 Oleg Fedorov
*/

#ifndef _FOS_GDT_H
#define _FOS_GDT_H

#include <types.h>

#define GDT_SIZE 8192*8-1

struct gdt_entry{
  volatile u8_t bytes[8];
}__attribute__ ((packed));

struct gdtr {
  volatile u16_t limit;
  volatile gdt_entry *base;
} __attribute__ ((packed));

class GDT {
 private:
  gdt_entry * gdt;
  volatile inline void lgdt(gdtr gdtr) { asm("lgdt %0"::"m"(gdtr)); }
  
 public:
  GDT();

  void load_tss(register num_t n, register gdt_entry * descr);
  void clear_descriptor(register num_t n);
  void set_tss_descriptor(register num_t n, register  off_t tss);
  void set_tss_descriptor(register off_t tss, register gdt_entry *descr);
  void set_call_gate(register u8_t n, register off_t offs, register u8_t dpl, register u8_t args);
  void set_segment_descriptor(register num_t n,
			    register u32_t base,
			    register u32_t limit,
			    register bool executable,
			    register bool writable,
			    register u8_t dpl,
			    register bool op32bit,
			    register bool conforming);
};

#endif
