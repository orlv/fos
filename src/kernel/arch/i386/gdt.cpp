/*
 * kernel/arch/i386/gdt.cpp
 * Copyright (C) 2005-2007 Oleg Fedorov
 */

#include <fos/mm.h>
#include "include/gdt.h"

GDT::GDT()
{
  gdtr gdtr;

  gdtr.limit = GDT_SIZE;
  gdt = (gdt_entry *) kmalloc(GDT_SIZE);
  gdtr.base =  (gdt_entry *) (u32_t)gdt;

  /* Нулевой дескриптор */
  clear_descriptor(0);

  /* (селектор 0x08)  0-4 Гб  Код ядра                */
  set_segment_descriptor(1, 0, 0xffffffff, 1, 0, 0, 1, 0);
  /* (селектор 0x10)  0-4 Гб  Данные ядра             */
  set_segment_descriptor(2, 0, 0xffffffff, 0, 1, 0, 1, 0);
  /* (селектор 0x1b)  0-4 Гб  Пользовательский код    */
  set_segment_descriptor(3, 0, 0xffffffff, 1, 0, 3, 1, 0);
  /* (селектор 0x23)  0-4 Гб  Пользовательские данные */
  set_segment_descriptor(4, 0, 0xffffffff, 0, 1, 3, 1, 0);

  /* (селектор 0x28) 16-битный код (для обращения к сервисам BIOS) */
  set_segment_descriptor(5, 0, 0xffff, 1, 1, 0, 0, 0);
  /* (селектор 0x30) 16-битные данные */
  set_segment_descriptor(6, 0, 0xffff, 0, 1, 0, 0, 0);

  lgdt(gdtr);
}

void GDT::set_segment_descriptor(num_t n,
				 u32_t base,
				 u32_t limit,
				 bool executable,
				 bool writable,
				 u8_t dpl,
				 bool op32bit,
				 bool conforming)
{
  gdt_entry *d = &gdt[n];
 
  bool gran4k = limit > 0xfffff;
  if(gran4k)
    limit /= 0x1000;

  d->bytes[0] = (limit & 0x000ff);
  d->bytes[1] = (limit & 0x0ff00) >> 8;
  d->bytes[6] = (limit & 0xf0000) >> 16;

  d->bytes[2] = base & 0x000000ff;
  d->bytes[3] = (base & 0x0000ff00) >> 8;
  d->bytes[4] = (base & 0x00ff0000) >> 16;
  d->bytes[7] = (base & 0xff000000) >> 24;

  d->bytes[5] = (dpl & 0x3) << 5;
  d->bytes[5] |= executable << 3;
  d->bytes[5] |= conforming << 2;
  d->bytes[5] |= writable  << 1;
  d->bytes[5] |= 0x80 | 0x10;

  d->bytes[6] |= (gran4k << 3 | op32bit << 2 ) << 4;
}


void GDT::clear_descriptor(num_t n)
{
  gdt_entry *d = &gdt[n];

  int i;
  for(i=0; i<8; i++){
    d->bytes[i] = 0;
  }
}

void GDT::set_tss_descriptor(num_t n, off_t tss)
{
  gdt_entry *descr = &gdt[n];
  set_tss_descriptor(tss, descr);
}

void GDT::set_tss_descriptor(off_t tss, gdt_entry *descr)
{
  descr->bytes[0] = 0x67;
  descr->bytes[1] = 0;

  descr->bytes[2] =  tss & 0x000000ff;
  descr->bytes[3] = (tss & 0x0000ff00) >> 8;
  descr->bytes[4] = (tss & 0x00ff0000) >> 16;
  descr->bytes[7] = (tss & 0xff000000) >> 24;

  descr->bytes[5] = 0x89;
  descr->bytes[6] = 0;
}


void GDT::load_tss(num_t n, gdt_entry * descr)
{
  gdt_entry *d = &gdt[n];

  int i;
  for(i=0; i<8; i++){
    d->bytes[i] = descr->bytes[i];
  }
}

void GDT::set_call_gate(u8_t n, off_t offs, u8_t dpl, u8_t args)
{
  struct g{
    u32_t a, b;
  }__attribute__ ((packed));

  volatile struct g *g = (struct g *)&gdt[n];
  
  g->a = 0x00080000 | (offs & 0xffff);
  g->b = (offs & 0xffff0000) | 0x8c00 | (dpl << 13) | (args & 0x1f);
}
