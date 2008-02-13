/*
  fos/system.h
  Copyright (C) 2007-2008 Oleg Fedorov
*/

#ifndef _FOS_SYSTEM_H
#define _FOS_SYSTEM_H

#include <types.h>
#include <multiboot.h>
#include <fos/gdt.h>
#include <fos/idt.h>
#include <fos/procman.h>
#include <fos/drivers/pic.h>
#include <fos/mm.h>
#include <fos/page.h>
#include <c++/stack.h>

static inline void preempt_reset()
{
  extern atomic_t preempt_count;
  preempt_count.set(1);
}
  
static inline void preempt_disable()
{
  extern atomic_t preempt_count;
  preempt_count.inc();
}

static inline void preempt_enable_no_resched()
{
  extern atomic_t preempt_count;
  if(preempt_count.value()) preempt_count.dec();
}

static inline void preempt_enable()
{
  extern atomic_t preempt_count;
  if(preempt_count.value()) preempt_count.dec();
  /*  if(!preempt_count.value() && пропущено_переключение)
      sched_yield();*/
}

static inline bool preempt_status()
{
  extern atomic_t preempt_count;
  return preempt_count.value() == 0;
}

static inline void preempt_on()
{
  extern atomic_t preempt_count;
  preempt_count.set(0);
}

class SYSTEM {
 private:
  multiboot_info_t *mbi;

 public:
  SYSTEM(register multiboot_info_t * mbi);
  
  TProcMan *procman;
  GDT *gdt;
  IDT *idt;
  PIC *pic;
  Thread ** user_int_handler;

  VMM *kmem;

  page *phys_page;        /* массив информации о страницах */
  size_t pages_cnt;       /* общее количество страниц в системе */
  Stack<u32_t> *free_page;
  atomic_t free_pages;    /* количество свободных страниц */
  Stack<u32_t> *free_page_DMA16;
  atomic_t free_pages_DMA16; /* количество "нижних" страниц (лежащих ниже 16 Мб) */

  inline void cli() { asm("cli"); };
  inline void sti() { asm("sti"); };

  struct {
    inline void reset() {
      preempt_reset();
    }

    inline void disable() {
      preempt_disable();
    }

    inline void enable() {
      preempt_enable();
    }

    inline bool status() {
      return preempt_status();
    }
  } preempt;
  
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

extern SYSTEM *system;

static inline int page_status(u32_t n)
{
  if(n < system->pages_cnt)
    return system->phys_page[n].mapcount.value();
  else
    return -1;
}

static inline u32_t kmem_log_addr(u32_t n)
{
  if(n < system->pages_cnt)
    return system->phys_page[n].kernel_map;
  else
    return 0;
}

static inline void kmem_set_log_addr(u32_t n, u32_t kmap_address)
{
  if(n < system->pages_cnt)
    system->phys_page[n].kernel_map = kmap_address;
}

static inline int alloc_page(u32_t n)
{
  if(n < system->pages_cnt)
    return system->phys_page[n].mapcount.inc_return();
  else
    return -1;
}

static inline int free_page(u32_t n)
{
  if(n < system->pages_cnt) {
    if(system->phys_page[n].mapcount.value())
      return system->phys_page[n].mapcount.dec_return();
    else
      return 0;
  } else
    return -1;
}

static inline Thread * THREAD(tid_t tid)
{
  return system->procman->task.tid->get(tid);
}

#endif
