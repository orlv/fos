/*
 * kernel/idt.cpp
 * Copyright (C) 2005-2007 Oleg Fedorov
 */

#include <idt.h>
#include <mm.h>


IDT::IDT()
{
  struct idtr idtr;

  idtr.limit = IDT_LIMIT;
  idt = idtr.base = (idt_entry *) kmalloc(IDT_LIMIT);
  
  lidt(idtr);
}

void IDT::set_trap_gate(u8_t n, off_t offs, u8_t dpl)
{
  idt[n].data[0] = 0x00080000 | (offs & 0xffff);
  idt[n].data[1] = (offs & 0xffff0000) | 0x8f00 | (dpl << 13);
}

void IDT::set_intr_gate(u8_t n, off_t offs)
{
  idt[n].data[0] = 0x00080000 | (offs & 0xffff);
  idt[n].data[1] = (offs & 0xffff0000) | 0x8e00;
}

void IDT::set_call_gate(u8_t n, off_t offs, u8_t dpl, u8_t args)
{
  idt[n].data[0] = 0x00080000 | (offs & 0xffff);
  idt[n].data[1] = (offs & 0xffff0000) | 0x8c00 | (dpl << 13) | (args & 0x1f);
}

void IDT::clear_descriptor(num_t n)
{
  idt[n].data[0] = 0;
  idt[n].data[1] = 0;
}
