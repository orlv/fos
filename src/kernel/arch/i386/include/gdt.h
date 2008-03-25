/*
    fos/arch/i386/include/gdt.h
    Copyright (C) 2006-2007 Oleg Fedorov
*/

#ifndef _GDT_H
#define _GDT_H

#include <types.h>

/* размер gdt - 64 килобайта */
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

  void load_tss(num_t n, gdt_entry * descr);
  void clear_descriptor(num_t n);
  void set_tss_descriptor(num_t n,  off_t tss);
  void set_tss_descriptor(off_t tss, gdt_entry *descr);
  void set_call_gate(u8_t n, off_t offs, u8_t dpl, u8_t args);
  void set_segment_descriptor(num_t n,
			    u32_t base,
			    u32_t limit,
			    bool executable,
			    bool writable,
			    u8_t dpl,
			    bool op32bit,
			    bool conforming);
};

#endif
