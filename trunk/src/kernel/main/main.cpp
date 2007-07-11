/*
 * kernel/main/main.cpp
 * Copyright (C) 2005-2007 Oleg Fedorov
 */

#include <multiboot.h>
#include <drivers/char/tty/tty.h>
#include <mm.h>
#include <drivers/block/vga/vga.h>
#include <string.h>
#include <system.h>
#include <stdio.h>
#include <drivers/char/timer/timer.h>
#include <hal.h>
#include <traps.h>
#include <vsprintf.h>
#include <stdarg.h>
#include <drivers/fs/modulefs/modulefs.h>
#include <fs.h>
#include <drivers/char/usertty/usertty.h>
#include <gdb.h>

TTY *stdout;
Timer *SysTimer;
HAL *hal;
ModuleFS *initrb; /* Модули, загруженные GRUB */

void out_banner()
{
  extern const string version;
  extern u32_t build;
  extern const string compile_date, compile_time;

  printk(" FOS OS. Revision %s. Build #%d %s %s \n"			\
	 "--------------------------------------------------------------------------------" \
	 " Copyright (C) 2004-2007 Oleg Fedorov                      http://fos.osdev.ru/ " \
	 "--------------------------------------------------------------------------------",
	 version, build, compile_date, compile_time);
}

#ifdef __KERNEL_SERIAL

#define PORT 0x3f8   /* COM1 */

void init_serial() 
{
  hal->outportb(PORT + 1, 0x00);    // Disable all interrupts
  hal->outportb(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
  hal->outportb(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
  hal->outportb(PORT + 1, 0x00);    //                  (hi byte)
  hal->outportb(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
  hal->outportb(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
  hal->outportb(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

int c = 0;
void serial_put(char ch)
{
  if(c==0) 
    {
      init_serial(); 
      c = 1; 
    }
  while (hal->inportb(PORT + 5) & 0x20 == 0);
  int i; for(i=0;i<100;i++);
  hal->outportb(PORT, ch);
}

char serial_get() 
{
  while ((hal->inportb(PORT + 5) & 1) == 0);
  char ch = hal->inportb(PORT);
  return ch;
}

#endif

#ifdef __SERIAL_GDB_STUB
#ifndef __KERNEL_SERIAL
#error Serial GDB stub needs '___KERNEL_SERIAL'!
#endif

extern "C" int getDebugChar()
{
  return serial_get();
}

extern "C" void putDebugChar(int ch)
{
  serial_put(ch);
}

extern "C" void exceptionHandler (int exception_number, void *exception_address)
{
  hal->idt->set_trap_gate(exception_number, (u32_t)exception_address, 0);
}

#endif

#ifdef __GDB_STUB
extern "C" void set_debug_traps();
extern "C" void breakpoint();
#endif

asmlinkage tid_t exec(const char * filename)
{
  size_t len = strlen(filename);
  if(len+1 > MAX_PATH_LEN)
    return 0;
  message msg;
  msg.a0 = PROCMAN_CMD_EXEC;
  msg.send_buf = filename;
  msg.send_size = len + 1;
  msg.recv_size = 0;
  msg.tid = SYSTID_PROCMAN;
  return send(&msg);
}

asmlinkage void init()
{
  init_memory();

  hal->cli();
  hal->pic = new PIC;
  hal->pic->remap(0x20, 0x28);

  int i;
  for(i = 0; i < 16; i++)
    hal->pic->mask(i);

  hal->gdt = new GDT;
  hal->idt = new IDT;

  setup_idt();
  hal->sti();

  VGA *con = new VGA;
  TTY *tty1 = new TTY(80, 25);

  tty1->stdout = con;
  tty1->set_text_color(WHITE);

  stdout = tty1;

  out_banner();
  printk("Memory size: %d Kb, free %dK (%dK high/%dK low)\n", hal->pages_cnt*4, hal->free_pages.read()*4 + hal->free_lowpages.read()*4, hal->free_pages.read()*4, hal->free_lowpages.read()*4);

  //extern size_t volatile heap_free;
  //printk("Memory size: %d Kb, free %dK (%dK high/%dK low), heap_free=%d\n", hal->pages_cnt*4, atomic_read(&hal->free_pages)*4 + atomic_read(&hal->free_lowpages)*4, atomic_read(&hal->free_pages)*4, atomic_read(&hal->free_lowpages)*4, heap_free);

  hal->procman = new TProcMan;

#ifdef __GDB_STUB
  printk("Setting up a GDB stub... ");
  set_debug_traps();
  printk("OK\n");
  printk("Issuing a breakpoint... ");
  breakpoint();
  printk("OK\n");
#endif

  SysTimer = new Timer;

  printk("Kernel: start task switching\n");
  
  hal->mt_reset();  /* сбросим счетчик на 1 */
  hal->mt_enable();

  sched_yield();

  extern multiboot_info_t *__mbi;
  initrb = new ModuleFS(__mbi);

  printk("--------------------------------------------------------------------------------" \
	 "All OK. Main kernel procedure done.\n"			\
	 "--------------------------------------------------------------------------------");

  //  i=0;
  while(1){
    //i++;
    /*if(i == 0x150){
      printf("\nMemory size: %d Kb, free %dK (%dK high/%dK low), heap_free=%d ", hal->pages_cnt*4, atomic_read(&hal->free_pages)*4 + atomic_read(&hal->free_lowpages)*4, atomic_read(&hal->free_pages)*4, atomic_read(&hal->free_lowpages)*4, heap_free);
      i=0;
      }*/
    sched_yield();
  }
}
