/*
 * kernel/main/main.cpp
 * Copyright (C) 2005-2006 Oleg Fedorov
 */

#include <multiboot.h>
#include <drivers/char/tty/tty.h>
#include <mm.h>
#include <paging.h>
#include <drivers/block/vga/vga.h>
#include <string.h>
#include <system.h>
#include <stdio.h>
#include <drivers/char/timer/timer.h>
#include <hal.h>
#include <traps.h>
#include <vsprintf.h>
#include <stdarg.h>

//#include <drivers/fs/objectfs/objectfs.h>
//#include <fslayer.h>

//#include <drivers/char/cmos/cmos.h>
//#include <drivers/block/floppy/floppy.h>
//#include <drivers/block/hd/hd.h>
#include <drivers/fs/modulefs/modulefs.h>

#include <drivers/char/keyboard/keyboard.h>

//#include <elf32.h>

void halt();

TTY *stdout;

Keyboard *keyb;
TTime *SysTimer;

asmlinkage void keyboard_handler()
{
  keyb->handler();
};

static inline void EnableTimer()
{
  hal->outportb(0x21, hal->inportb(0x21) & 0xfe); /* Enable timer */
}

HAL *hal;

int printf(const char *fmt, ...)
{
  extern char printbuf[2000];
  int i = 0;
  va_list args;
  va_start(args, fmt);
  i = vsprintf(printbuf, fmt, args);
  va_end(args);

  printbuf[i] = 0;
  volatile struct message msg;
  msg.send_buf = msg.recv_buf = printbuf;
  msg.send_size = i + 1;
  msg.recv_size = 10;
  msg.pid = 2;
  syscall_send((struct message *)&msg);

  return i;
}

asmlinkage void init()
{
  extern const string version;
  extern u32_t build;
  extern const string compile_date, compile_time;

  extern multiboot_info_t *__mbi;

  init_memory();
  
  hal = new HAL(__mbi);
  
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
  tty1->SetTextColor(WHITE);

  stdout = tty1;

  *stdout << "FOS OS. Revision " << version << ". Build #" << build << " " << compile_date << " " << compile_time << "\n";

  printk("--------------------------------------------------------------------------------");
#if 1
#if 1
  hal->ProcMan = new TProcMan;
  SysTimer = new TTime;
  EnableTimer();
#endif

#if 0
  struct time clock;
  device->object->read(0, &clock, sizeof(struct time));
  device = dir->access("hwclock");
  printk("\nDate from /dev/hwclock: %04d-%02d-%02d, %02d:%02d:%02d\n",
	 clock.year, clock.month, clock.day, clock.hour, clock.min, clock.sec);
#endif

#if 1
  extern multiboot_info_t *__mbi;
  ModuleFS *modules = new ModuleFS(__mbi);
  /*
  obj_info_t *dirent;
  off_t i;
  for (i = 0; (dirent = modules->list(i)); i++) {
    if (dirent->info.type == FTypeDirectory)
      printk("drwxrwxrwx 1 root:root %6d %s\n", dirent->info.size,
	     dirent->name);
    else
      printk("-rwxrwxrwx 1 root:root %6d %s\n", dirent->info.size,
	     dirent->name);

    delete dirent;
  }
  */
  Tinterface *obj;

  obj = modules->access("tty");
  string elf_buf = new char[obj->info.size];
  obj->read(0, elf_buf, obj->info.size);
  hal->ProcMan->exec(elf_buf);
  delete elf_buf;
    
  obj = modules->access("fs");
  elf_buf = new char[obj->info.size];
  obj->read(0, elf_buf, obj->info.size);
  hal->ProcMan->exec(elf_buf);
  delete elf_buf;
#endif

#if 0
  Tdirectory *dir;
  Tobject *obj;

  printk("Setting up ObjectFS && FSLayer..");
  dir = fs = new Tdirectory(new ObjectFS(0), 0);
  printk(" OK\n");

  dir->construct("dev", FTypeDirectory, 0);

  /* открываем каталог "/dev" */
  obj = dir->access("dev");
  dir = obj->directory;

  dir->construct("tty1", FTypeObject, tty1);
  dir->construct("con", FTypeObject, con);

  CMOS *cmos = new CMOS;
  dir->construct("hwclock", FTypeObject, cmos);

  ls(dir);

  //  Tobject *device;
  //  device = dir->access("tty1");
  //  device->object->write(0, "test", 4);
#endif

#if 0
  /* проверено. в билде 9936 - работало */
  printk("Setting up /dev/fda... ");
  floppy *fda = new floppy;
  dir->construct("fda", FTypeObject, fda);
#endif

#if 0
  keyb = new Keyboard;
  dir->construct("keyboard", FTypeObject, keyb);
#endif

#if 0
  Tobject *keyb;
  keyb = dir->access("keyboard");
#endif

#if 0
  Tobject *device;
  /* проверено. в билде 9936 - работало */
  string buf = new char[512];

  device = dir->access("fda");
  printk("\nReading sector 1 from /dev/fda: \n");
  device->object->read(0, buf, 512);
  int i;
  printk("---------------------------------\n");
  for (i = 0; i < 512; i++)
    printk("%c", buf[i]);
  printk("\n---------------------------------\n");
  delete buf;
#endif

#if 0
  /* работает :) */
  printk("Setting up /dev/hda... ");
  hd *hda = new hd;
  dir->construct("hda", FTypeObject, hda);
#endif

#if 0
  string buf = new char[512];
  device = dir->access("hda");
  printk("\nReading sector 0 from /dev/hda: \n");
  device->object->read(0, buf, 512);

  int i;
  printk("---------------------------------\n");
  for (i = 0; i < 512; i++)
    printk("%c", buf[i]);
  printk("\n---------------------------------\n");
  delete buf;
#endif

#if 0
  vesa_test();
  int offs;
  for (offs = 0; offs < 640 * 480; offs++) {
    putpixel(offs, 0xffff);
  }
#endif

  //  Tdirectory *dir;
  //  Tobject *obj;

  /*  dir = fs;
     obj = dir->access("rd");
     dir = obj->directory;

     obj = dir->access("pic.bmp"); */
  //  string buf = new char [obj->object->info.size];

  //u32_t * buf1 = (u32_t *) 0xf0000000;
  //  string buf = (string) new char[4096];
  //  obj->object->read(0, buf1, obj->object->info.size);

#if 0
  //  elf_test("tty");
  //  elf_test("app1");
#endif

#if 0
  u32_t x = 0, y = 0;
  u32_t size = 100;
  u16_t color = 0xaabb;

  sti();
  u8_t *kbuf = new u8_t[10];
  while (1) {
    keyb->object->read(0, kbuf, 1);

    if (kbuf[0] == 75) {
      hide_bar(x, y, size);
      x -= size;
      if (x < 0)
	x = 0;
      bar(x, y, size, color);
    } else if (kbuf[0] == 77) {
      hide_bar(x, y, size);
      x += size;
      if (x + size > 639)
	x = 0;

      bar(x, y, size, color);
    } else if (kbuf[0] == 72) {
      hide_bar(x, y, size);
      y -= size;
      if (y < 0)
	y = 0;
      bar(x, y, size, color);
    } else if (kbuf[0] == 80) {
      hide_bar(x, y, size);
      y += size;
      if (y + size > 479)
	y = 0;

      bar(x, y, size, color);
    }

    /*      putpixel(x+y*640,color);
       x++;
       if(x > 639) {x = 0; y++;}
       if(y > 479) y = 0; */
    color += x + y;
  }
  /*  for(y=5; y<465; y+=size)
     for(x=5; x<625; x+=size)
     {
     bar(x,y,size);
     hide_bar(x,y,size);
     } */
  //  x = 60;
  //  y = 3;
  //  offs = x + y*640;
#endif

#endif
  
  printf("\n--------------------------------------------------------------------------------" \
	 "All OK. Init done.\n" \
	 "You can get new version at http://fos.osdev.ru/" \
	 "\n--------------------------------------------------------------------------------");


  char *filename = new char[256];
  struct message *msg = new message;;

  //  printf("init pid=%d", hal->ProcMan->CurrentProcess->pid);
  
  while (1) {
    asm("incb 0xb8000+154\n" "movb $0x5e,0xb8000+155 ");

    msg->pid = 0;
    msg->recv_size = 256;
    msg->recv_buf = filename;
    syscall_receive(msg);

    printf("\nProcMan: exec %s\n", filename);

    if(obj = modules->access(filename)){
      elf_buf = new char[obj->info.size];
      obj->read(0, elf_buf, obj->info.size);
      hal->ProcMan->exec(elf_buf);
      delete elf_buf;
      strcpy(filename, "OK");
    } else {
      strcpy(filename, "ER");
    }

    msg->recv_size = 0;
    msg->send_size = 3;

    filename[2] = 0;
    msg->send_buf = filename;
    syscall_reply(msg);

    //keyb->object->read(0, kbuf, 1);
    //printk("[%d]",kbuf[0]);
  }

#if 0
  /*
   *  Halt process until exception appears
   */
  sti();
  while (1)
    hlt();
#endif
}
