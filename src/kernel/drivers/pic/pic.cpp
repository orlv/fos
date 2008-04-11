/*
  drivers/pic.cpp
  Copyright (C) 2007 Oleg Fedorov
*/

#include "pic.h"
#include <fos/system.h>
#include <fos/printk.h>

PIC::PIC()
{
  remap(0x20, 0x28);
  printk("PIC: Configured for base 0x20\n");
}

void PIC::remap(u8_t v1, u8_t v2)
{
  v1 &= 0xF8;
  v2 &= 0xF8;
  system->outportb(0x20, 0x11);
  system->outportb(0xA0, 0x11);
  system->outportb(0x21, v1);
  system->outportb(0xA1, v2);
  system->outportb(0x21, 0x04);
  system->outportb(0xA1, 0x02);
  system->outportb(0x21, 0x01);
  system->outportb(0xA1, 0x01);

  /* запретим прерывания, кроме каскадируемого */
  system->outportb(0xA1, 0xff);
  system->outportb(0x21, 0xfb);
}

/* запрещает IRQ номер n */
void PIC::mask(int n)
{
  if (n > 15)
    system->panic("PIC::mask(): n > 15!");

  u8_t port;
  
  if (n > 7) {
    n -= 8;
    port = 0xA1;
  } else
    port = 0x21;

  system->outportb(port, system->inportb(port) | (1<<n));
}

/* разрешает IRQ номер n */
void PIC::unmask(int n)
{
  if(n > 15)
    system->panic("PIC::unmask(): n > 15!");

  u8_t port;
  
  if (n > 7) {
    n -= 8;
    port = 0xa1;
  } else {
    port = 0x21;
  }

  system->outportb(port, system->inportb(port) & ~(1<<n));
}

void PIC::lock()
{
  status = (system->inportb(0xa1) << 8) | system->inportb(0x21);
  system->outportb(0xA1, 0xff);
  system->outportb(0x21, 0xfb);
}

void PIC::unlock()
{
  system->outportb(0xa1, status >> 8);
  system->outportb(0x21, status & 0xff);
}

void PIC::Route(int irq)
{
  if(irq > 15)
    system->panic("PIC::Route(): irq > 15!");

  if(handlers[irq]) {
    (handlers[irq])();
  } else if(system->user_int_handler[irq]) {
    system->user_int_handler[irq]->put_signal(irq, SIGNAL_IRQ);
    EOI(irq);
  } else if(irq == 7 || irq == 15) {
    printk("PIC: Spurious interrupt %d\n", irq);
    EOI(irq);
  } else 
    system->panic("PIC: Unhandled interrupt %d\n", irq);
}

void PIC::EOI(int irq)
{
  if(irq > 7)
    system->outportb(0xa0, 0x20);

  system->outportb(0x20, 0x20);
}

void PIC::setHandler(int n, void *handler)
{
  if(n > 15) 
    system->panic("PIC::setHandler: irq > 15!");

  handlers[n] = (void (*)()) handler;
  printk("PIC: handler of irq %d set to 0x%X\n", n, handler);
}

void *PIC::getHandler(int n)
{
  if(n > 15) 
     system->panic("PIC::getHandler: irq > 15!");

  return (void *)handlers[n];
}

Timer *PIC::getTimer()
{
  system->panic("PIC has no timer!\n");
  return NULL;
}
