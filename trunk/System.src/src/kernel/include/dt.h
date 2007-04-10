/*
    kernel/include/dt.h
    Copyright (C) 2006 Oleg Fedorov
*/

#ifndef _DT_H
#define _DT_H

#include <types.h>
#include <system.h>
#include <mm.h>

struct dt_t {
  u32_t a, b;
};

class DTMan {
  struct dt_t *gdt;
  struct dt_t *idt;

public:
   DTMan();
  u32_t new_gdt();
  void delete_gdt(u16_t n);
  void load_tss(u32_t n, dt_t * descr);
  void set_trap_gate(u16_t n, off_t offs, u8_t dpl);
  void set_intr_gate(u16_t n, off_t offs);
  void set_call_gate(u16_t n, off_t offs, u8_t dpl, u8_t args);
};

extern DTMan *DTman;

void setup_tss(dt_t * descr, off_t ptr);

#endif
