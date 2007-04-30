/*
  kernel/include/idt.h
  Copyright (C) 2006-2007 Oleg Fedorov
*/

#ifndef _IDT_H
#define _IDT_H

#include <types.h>

#define IDT_LIMIT 256*8-1

struct idt_entry{
  u32_t data[2];
}__attribute__ ((packed));

struct idtr {
  u16_t limit;
  idt_entry *base;
} __attribute__ ((packed));


class IDT {
 private:
  idt_entry * idt;
  volatile inline void lidt(idtr idtr){ asm("lidt %0"::"m"(idtr)); };
  
 public:
  IDT();

  void set_trap_gate(u8_t n, off_t offs, u8_t dpl);
  void set_intr_gate(u8_t n, off_t offs);
  void set_call_gate(u8_t n, off_t offs, u8_t dpl, u8_t args);
  void clear_descriptor(num_t n);  
};

#endif
