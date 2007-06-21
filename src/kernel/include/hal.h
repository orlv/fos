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

class HAL {
 private:
  multiboot_info_t *mbi;

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

  page *phys_page;      /* массив информации о страницах */
  size_t pages_cnt;     /* общее количество страниц в системе */
  memstack *free_page;  /* пул свободных страниц */
  atomic_t free_pages;  /* количество свободных страниц */
  memstack *free_lowpage;
  atomic_t free_lowpages;

  inline void cli() { asm("cli"); };
  inline void sti() { asm("sti"); };

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
    return atomic_read(&hal->phys_page[n].mapcount);
  else
    return -1;
}

static inline u32_t alloc_page(u32_t n)
{
  if(n < hal->pages_cnt)
    return atomic_inc_return(&hal->phys_page[n].mapcount);
  else
    return 0;
}

static inline u32_t free_page(u32_t n)
{
  if(n < hal->pages_cnt)
    return atomic_dec_return(&hal->phys_page[n].mapcount);
  else
    return 0;
}


#endif