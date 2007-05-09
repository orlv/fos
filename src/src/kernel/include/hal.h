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
#include <tasks.h>
#include <drivers/pic.h>

class HAL {
 private:
  multiboot_info_t *mbi;

 public:
  HAL(register multiboot_info_t * mbi);
  
  TProcMan *ProcMan;
  GDT *gdt;
  IDT *idt;
  PIC *pic;

  //  TerminalDriver *terminal;
  //  MemoryManager *mm;

  
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
};

extern HAL *hal;

#endif
