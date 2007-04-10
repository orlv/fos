/*
 * kernel/memory/dt.cpp
 * Copyright (C) 2005-2006 Oleg Fedorov
 */

#include <dt.h>
#include <mm.h>

/* загрузим gdtr */
#define lgdt(gdtr) asm("lgdt %0"::"m"(gdtr))

/* загрузим idtr */
#define lidt(idtr) asm("lidt %0"::"m"(idtr))

struct dtr {
  u16_t limit;
  dt_t *base;
} __attribute__ ((packed));

#define GDT_LIMIT 8192*8-1
#define IDT_LIMIT 256*8-1

asmlinkage void set_dt();

DTMan::DTMan()
{
  struct dtr gdtr;
  struct dtr idtr;

  gdtr.limit = GDT_LIMIT;
  gdt = gdtr.base = (dt_t *) kmalloc(GDT_LIMIT);

  idtr.limit = IDT_LIMIT;
  idt = idtr.base = (dt_t *) kmalloc(IDT_LIMIT);

  gdt[0].a = 0;
  gdt[0].b = 0;

  gdt[1].a = 0x0000ffff;	/* (0x08) Kernel 0-4 Гб код    */
  gdt[1].b = 0x00cf9a00;

  gdt[2].a = 0x0000ffff;	/* (0x10) Kernel 0-4 Гб данные */
  gdt[2].b = 0x00cf9200;

  gdt[3].a = 0x0000ffff;	/* (0x1b) User 0-4 Гб код      */
  gdt[3].b = 0x00cffa00;

  gdt[4].a = 0x0000ffff;	/* (0x23) User 0-4 Гб данные   */
  gdt[4].b = 0x00cff200;

  gdt[5].a = 0x0000ffff;	/* 0x28 */
  gdt[5].b = 0x00009e00;

  gdt[6].a = 0x0000ffff;	/* 0x30 */
  gdt[6].b = 0x00009200;

  lgdt(gdtr);
  lidt(idtr);
}

u32_t DTMan::new_gdt()
{
  for (u16_t n = BASE_TSK_SEL_N + 1; n < GDT_DESCR; n++)
    if (!gdt[n].a)
      return n;

  return 0;
}

void DTMan::delete_gdt(u16_t n)
{
  gdt[n].a = 0;
  gdt[n].b = 0;
}

/*
void DTMan::set_tss(u16_t n, u32_t ptr)
{
  gdt[n].a = (ptr<<16) | 0x67;
  gdt[n].b = (ptr & 0xff000000) | 0x00008900 | ((ptr >> 16) & 0xff);
}
*/

void DTMan::load_tss(u32_t n, dt_t * descr)
{
  gdt[n].a = descr->a;
  gdt[n].b = descr->b;
}

//#include <stdio.h>

void setup_tss(dt_t * descr, off_t ptr)
{
  descr->a = (ptr << 16) | 0x67;
  descr->b = (ptr & 0xff000000) | 0x00008900 | ((ptr >> 16) & 0xff);
  //printk("[0x%X:0x%X]", descr->a, descr->b);
}

void DTMan::set_trap_gate(u16_t n, off_t offs, u8_t dpl)
{
  idt[n].a = 0x00080000 | (offs & 0xffff);
  idt[n].b = (offs & 0xffff0000) | 0x8f00 | (dpl << 13);
}

void DTMan::set_intr_gate(u16_t n, off_t offs)
{
  idt[n].a = 0x00080000 | (offs & 0xffff);
  idt[n].b = (offs & 0xffff0000) | 0x8e00;
}

void DTMan::set_call_gate(u16_t n, off_t offs, u8_t dpl, u8_t args)
{
  gdt[n].a = 0x00080000 | (offs & 0xffff);
  gdt[n].b = (offs & 0xffff0000) | 0x8c00 | (dpl << 13) | (args & 0x1f);
}
