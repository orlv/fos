/*
 * kernel/main/idt.cpp
 * Copyright (C) 2005-2007 Oleg Fedorov
 */

#include <fos/idt.h>
#include <fos/mm.h>

IDT::IDT()
{
  struct idtr idtr;
  idtr.base = idt = (idt_entry *) kmalloc(IDT_SIZE);
  idtr.limit = IDT_SIZE;
  lidt(idtr);
}

void IDT::set_trap_gate(register u8_t n, register off_t offs, register u8_t dpl)
{
  idt[n].data[0] = 0x00080000 | (offs & 0xffff);
  idt[n].data[1] = (offs & 0xffff0000) | 0x8f00 | (dpl << 13);
}

void IDT::set_intr_gate(register u8_t n, register off_t offs)
{
  idt[n].data[0] = 0x00080000 | (offs & 0xffff);
  idt[n].data[1] = (offs & 0xffff0000) | 0x8e00;
}

void IDT::clear_descriptor(register num_t n)
{
  idt[n].data[0] = 0;
  idt[n].data[1] = 0;
}
