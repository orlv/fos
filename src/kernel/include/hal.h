/*
  kernel/include/hal.h
  Copyright (C) 2007 Oleg Fedorov
*/

#ifndef _HAL_H
#define _HAL_H

#include <types.h>
#include <multiboot.h>
#include <gdt.h>
#include <idt.h>
#include <procman.h>
#include <drivers/pic.h>
#include <namer.h>
#include <mm.h>
#include <stack.h>

static inline void __mt_reset()
{
  extern atomic_t mt_state;
  mt_state.set(1);
}
  
static inline void __mt_disable()
{
  extern atomic_t mt_state;
  mt_state.inc();
}

static inline void __mt_enable()
{
  extern atomic_t mt_state;
  if(mt_state.read()) mt_state.dec();
}

static inline bool __mt_status()
{
  extern atomic_t mt_state;
  return mt_state.read() == 0;
}


class HAL {
 private:
  multiboot_info_t *mbi;

  atomic_t interrupts;
  
 public:
  HAL(register multiboot_info_t * mbi);
  
  TProcMan *ProcMan;
  Namer *namer;
  tid_t tid_namer;
  GDT *gdt;
  IDT *idt;
  PIC *pic;
  u32_t *user_int_handler;

  Memory *kmem;

  page *phys_page;        /* массив информации о страницах */
  size_t pages_cnt;       /* общее количество страниц в системе */
  Stack<u32_t> *free_page;
  atomic_t free_pages;    /* количество свободных страниц */
  Stack<u32_t> *free_lowpage;
  atomic_t free_lowpages; /* количество "нижних" страниц (лежащих ниже 16 Мб) */

  inline void cli() { asm("cli"); };
  inline void sti() { asm("sti"); };

  inline void mt_reset()
  {
    __mt_reset();
  }
  
  inline void mt_disable()
  {
    __mt_disable();
  }

  inline void mt_enable()
  {
    __mt_enable();
  }

  inline bool mt_status()
  {
    return __mt_status();
  }
  
  inline void hlt() { asm("hlt"); };
  
  inline void outportb(u16_t port, u8_t value){
    asm volatile ("outb %0,%1"::"a" (value), "d"(port));
  }

  inline u8_t inportb(u16_t port){
    u8_t value;
    asm volatile ("inb %1, %0":"=a" (value):"d"(port));
    return value;
  }

  inline void outportw(u16_t port, u16_t value){
    asm volatile ("outw %0,%1"::"a" (value), "d"(port));
  }

  inline u16_t inportw(u16_t port){
    u16_t value;
    asm volatile ("inw %1, %0":"=a" (value):"d"(port));
    return value;
  }
  
  void panic(register const char *fmt, ...);
  void halt();

  res_t interrupt_attach(Thread *thread, u8_t n);
  res_t interrupt_detach(Thread *thread, u8_t n);
};

extern HAL *hal;

static inline int page_status(u32_t n)
{
  if(n < hal->pages_cnt)
    return hal->phys_page[n].mapcount.read();
  else
    return -1;
}

static inline u32_t kmem_log_addr(u32_t n)
{
  if(n < hal->pages_cnt)
    return hal->phys_page[n].kernel_map;
  else
    return 0;
}

static inline void kmem_set_log_addr(u32_t n, u32_t kmap_address)
{
  if(n < hal->pages_cnt)
    hal->phys_page[n].kernel_map = kmap_address;
}

static inline u32_t alloc_page(u32_t n)
{
  if(n < hal->pages_cnt)
    return hal->phys_page[n].mapcount.inc_return();
  else
    return 0;
}

static inline u32_t free_page(u32_t n)
{
  if(n < hal->pages_cnt)
    return hal->phys_page[n].mapcount.dec_return();
  else
    return 0;
}


#endif
