/*
 * kernel/gdt.cpp
 * Copyright (C) 2005-2007 Oleg Fedorov
 */

#include <gdt.h>
#include <mm.h>

GDT::GDT()
{
  gdtr gdtr;

  gdtr.limit = GDT_LIMIT;
  gdt = gdtr.base = (gdt_entry *) kmalloc(GDT_LIMIT);

  /* null descriptor */
  clear_descriptor(0);
  /* (0x08) Kernel 0-4 Гб код   */
  set_segment_descriptor(1, 0, 0xffffffff, 1, 0, 0, 1, 0);
  /* (0x10) Kernel 0-4 Гб данные   */
  set_segment_descriptor(2, 0, 0xffffffff, 0, 1, 0, 1, 0);
  /* (0x1b) User 0-4 Гб код      */
  set_segment_descriptor(3, 0, 0xffffffff, 1, 0, 3, 1, 0);
  /* (0x23) User 0-4 Гб данные   */
  set_segment_descriptor(4, 0, 0xffffffff, 0, 1, 3, 1, 0);

  /* 0x28 */
  set_segment_descriptor(5, 0, 0xffff, 1, 1, 0, 0, 1); 
  /* 0x30 */
  set_segment_descriptor(6, 0, 0xffff, 0, 1, 0, 0, 0);
  
  lgdt(gdtr);
}

void GDT::set_segment_descriptor(register num_t n,
				 register u32_t base,
				 register u32_t limit,
				 register bool executable,
				 register bool writable,
				 register u8_t dpl,
				 register bool op32bit,
				 register bool conforming)
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
  d->bytes[5] |= 0x80 | 0x10; // present application descriptor

  d->bytes[6] |= (gran4k << 3 | op32bit << 2 ) << 4;
}


void GDT::clear_descriptor(register num_t n)
{
  gdt_entry *d = &gdt[n];

  int i;
  for(i=0; i<8; i++){
    d->bytes[i] = 0;
  }
}

void GDT::set_tss_descriptor(register num_t n, register off_t tss)
{
  gdt_entry *descr = &gdt[n];
  set_tss_descriptor(tss, descr);
}

void GDT::set_tss_descriptor(register off_t tss, register gdt_entry *descr)
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


void GDT::load_tss(register num_t n, register gdt_entry * descr)
{
  gdt_entry *d = &gdt[n];

  int i;
  for(i=0; i<8; i++){
    d->bytes[i] = descr->bytes[i];
  }
}

void GDT::set_call_gate(register u8_t n, register off_t offs, register u8_t dpl, register u8_t args)
{
  struct g{
    u32_t a, b;
  }__attribute__ ((packed));

  volatile struct g *g = (struct g *)&gdt[n];
  
  g->a = 0x00080000 | (offs & 0xffff);
  g->b = (offs & 0xffff0000) | 0x8c00 | (dpl << 13) | (args & 0x1f);
}
