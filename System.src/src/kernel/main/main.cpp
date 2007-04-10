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
#include <tasks.h>
#include <system.h>
#include <stdio.h>
#include <drivers/char/timer/timer.h>
#include <dt.h>

#include <drivers/fs/objectfs/objectfs.h>
#include <fslayer.h>

#include <drivers/char/cmos/cmos.h>
#include <drivers/block/floppy/floppy.h>
#include <drivers/block/hd/hd.h>
#include <drivers/fs/modulefs/modulefs.h>

#include <drivers/char/keyboard/keyboard.h>

#include <elf32.h>

#include <io.h>

void halt();

TTY *stdout;
TProcMan *ProcMan;
DTMan *DTman;
TTime *SysTimer;
Keyboard *keyb;
Tdirectory *fs;

asmlinkage void keyboard_handler()
{
  keyb->handler();
};

#define sys_call(arg1,arg2,arg3, arg4) asm("int $0x30"::"a"(arg1), "b"(arg2),"c"(arg3),"d"(arg4))

/*
void foo_call()
{
  sys_call(1,2,3,4);
}
*/

static inline void ls(Tdirectory * dir)
{
  obj_info_t *dirent;
  off_t i;
  for (i = 0; (dirent = dir->list(i)); i++) {
    if (dirent->info.type == FTypeDirectory)
      printk("drwxrwxrwx 1 root:root %6d %s\n", dirent->info.size,
	     dirent->name);
    else
      printk("-rwxrwxrwx 1 root:root %6d %s\n", dirent->info.size,
	     dirent->name);

    delete dirent;
  }
}

struct Tsection {
  off_t start;
  size_t size;
  void *data;
};

#if 0
u32_t load_elf(const string name)
{
  Tdirectory *dir;
  Tobject *obj;

  dir = fs;
  obj = dir->access("rd");
  dir = obj->directory;

  //  ls(dir);

  obj = dir->access(name);
  string buf = new char[obj->object->info.size];

  obj->object->read(0, buf, obj->object->info.size);

  Elf32_Phdr *p;
  u32_t i;
  Elf32_Ehdr *h = (Elf32_Ehdr *) buf;
  Elf32_Phdr *ph = (Elf32_Phdr *) ((u32_t) h + (u32_t) h->e_phoff);
  u32_t *object, *filedata;

  /* printk("ident=%s \n" \
     "type=%d \n" \
     "machine=%d \n" \
     "version=%d \n" \
     "entry=0x%X \n" \
     "phoff=0x%X \n" \
     "shoff=0x%X \n" \
     "flags=%d \n" \
     "ehsize=%d \n" \
     "phentsize=%d \n" \
     "phnum=%d \n" \
     "shentsize=%d \n" \
     "shnum=%d \n" \
     "shstrndx=%d \n",
     h->e_ident,h->e_type,h->e_machine,h->e_version,h->e_entry,h->e_phoff,
     h->e_shoff,h->e_flags,h->e_ehsize,h->e_phentsize,h->e_phnum,
     h->e_shentsize,h->e_shnum,h->e_shstrndx);
   */
  TList *sections = new TList;
  Tsection *section;

  /*  TListEntry *entry = sections->FirstEntry;
     for(i=0; i < sections->count; i++)
     {
     section = (Tsection *)entry->data;
     printk("[0x%X]", section->size);
     int j;

     for(j=0; j < section->size/4; j++)
     {
     printk("%X ", ((u32_t *)section->data)[j]);
     }
     entry=entry->next;
     }
   */
  //  while(1);

  Elf32_Shdr *sh;
  u32_t text, data, bss;
  string st;
  Elf32_Sym *sym;
  Elf32_Rel *rel;
  size_t b_size = 0;
  u32_t r_cnt = 0, sym_cnt = 0;

  sh = (Elf32_Shdr *) ((u32_t) h + (u32_t) h->e_shoff);
  st = (string) ((u32_t) h + (u32_t) sh[h->e_shstrndx].sh_offset);

  for (i = 1; i < h->e_shnum; i++) {
#if 0
    printk
	("[%s], n=0x%X, type=0x%X, flags=0x%X, laddr=0x%X, foffset=0x%X, size=0x%X\n",
	 (string) (st + sh[i].sh_name), sh[i].sh_name, sh[i].sh_type,
	 sh[i].sh_flags, sh[i].sh_addr, sh[i].sh_offset, sh[i].sh_size);
#endif
    if (sh[i].sh_type == 0x09) {
      rel = (Elf32_Rel *) ((u32_t) h + (u32_t) sh[i].sh_offset);
      r_cnt = sh[i].sh_size / sizeof(Elf32_Rel);
    }
    if (sh[i].sh_type == 0x0b) {
      sym = (Elf32_Sym *) ((u32_t) h + (u32_t) sh[i].sh_offset);
      sym_cnt = sh[i].sh_size / sizeof(Elf32_Sym);
    }
    if (sh[i].sh_type == 0x08) {
      b_size = sh[i].sh_size;
    }
  }

  for (p = ph; p < ph + h->e_phnum; p++) {
    /* Если секция загружаемая и займёт сколько-нибудь памяти.. */
    if (p->p_type == ELF32_TYPE_LOAD && p->p_memsz) {
      printk
	  ("flags=%d, addr=0x%X, type=%d, fileoffs=0x%X, filesz=0x%X, memsz=0x%X \n",
	   p->p_flags, p->p_vaddr, p->p_type, p->p_offset, p->p_filesz,
	   p->p_memsz);

      if (p->p_filesz > p->p_memsz) {
	printk("Invalid section!");
	delete buf;
	panic("666");
      }

      section = new Tsection;
      section->data = new u32_t[p->p_memsz / sizeof(u32_t)];
      object = (u32_t *) section->data;
      section->size = p->p_memsz;
      section->start = p->p_vaddr;

      sections->insert(section);
      /* Копируем данные */
      filedata = (u32_t *) (buf + p->p_offset);
      //      printk("fsz=0x%X, memsz=0x%X\n", p->p_filesz, p->p_memsz);

      for (i = 0; i < p->p_filesz / sizeof(u32_t) + 1; i++) {
	object[i] = filedata[i];
      }
    }
  }

  if (b_size) {
    bss = (u32_t) new u8_t[b_size];
    //      section = new Tsection;
    //      section->size = b_size;
    //      sections->insert(section);
  }
#if 0
  for (i = 0; i < r_cnt; i++) {
    off_t offs = rel[i].r_offset;
    u32_t type = ELF32_R_TYPE(rel[i].r_info);
    printk("rel type %d, offs=0x%X, value=0x%X \n", type, offs,
	   ELF32_R_SYM(rel[i].r_info));

    TListEntry *entry = sections->FirstEntry;
    //      int k;
    //      for(k=0; k < sections->count; k++)
    //        {
    section = (Tsection *) entry->data;
    //printk("[0x%X]", section->size);
    //      if(offs > section->start && offs < section->size)
    //        {
    if (type == 0x02) {
      *(u32_t *) ((u32_t) section->data + offs) =
	  (u32_t) ((u32_t) sym[ELF32_R_SYM(rel[i].r_info)].st_value - offs);
    }
    if (type == 0 && b_size)
      ((u32_t *) section->data)[offs / 4] = bss;
    //              break;
    //            }
    //      entry=entry->next;
    //    }

  }
#endif
  /*
     for(i=0; i < sym_cnt; i++)
     {
     if(sym[i].st_info == 0x12)
     {
     printk("name=0x%X, value=0x%X, size=0x%X, info=0x%X, ndx=0x%X, rndx=0x%X\n", sym[i].st_name, sym[i].st_value, sym[i].st_size, sym[i].st_info, sym[i].st_shndx, i);
     }
     }
   */

  u32_t code =
      (u32_t) ((u32_t) ((Tsection *) sections->FirstEntry->data)->data +
	       h->e_entry);
  return code;
}
#endif

void elf_test(string name)
{
#if 1
  //  u32_t entry = load_elf(name);
  //  asm volatile("call *%%eax"::"a"(entry));

  Tdirectory *dir;
  Tobject *obj;

  dir = fs;
  obj = dir->access("rd");
  dir = obj->directory;

  obj = dir->access(name);

  string elf_buf = new char[obj->object->info.size];

  obj->object->read(0, elf_buf, obj->object->info.size);

  ProcMan->exec(elf_buf);
#endif
};

u32_t load_binary(const string name)
{
  Tdirectory *dir;
  Tobject *obj;

  dir = fs;
  obj = dir->access("rd");
  dir = obj->directory;

  obj = dir->access(name);
  //  string buf = new char [obj->object->info.size];

  string buf = (string) 0x1f00;
  obj->object->read(0, buf, obj->object->info.size);

  return (u32_t) buf;
}

struct vbe_info_block {
  u8_t signature[4];
  u16_t version;
  u32_t oem_string_ptr;
  u32_t capabilities;
  u32_t video_mode_ptr;
  u16_t total_memory;

  u16_t oem_software_rev;
  u32_t oem_vendor_name_ptr;
  u32_t oem_product_name_ptr;
  u32_t oem_product_rev_ptr;

  u8_t reserved[222];

  u8_t oem_data[256];
} __attribute__ ((packed));

struct vbe_mode_info_block {
  /* Mandory information for all VBE revisions.  */
  u16_t mode_attributes;
  u8_t win_a_attributes;
  u8_t win_b_attributes;
  u16_t win_granularity;
  u16_t win_size;
  u16_t win_a_segment;
  u16_t win_b_segment;
  u32_t win_func_ptr;
  u16_t bytes_per_scan_line;

  /* Mandory information for VBE 1.2 and above.  */
  u16_t x_resolution;
  u16_t y_resolution;
  u8_t x_char_size;
  u8_t y_char_size;
  u8_t number_of_planes;
  u8_t bits_per_pixel;
  u8_t number_of_banks;
  u8_t memory_model;
  u8_t bank_size;
  u8_t number_of_image_pages;
  u8_t reserved;

  /* Direct Color fields (required for direct/6 and YUV/7 memory models).  */
  u8_t red_mask_size;
  u8_t red_field_position;
  u8_t green_mask_size;
  u8_t green_field_position;
  u8_t blue_mask_size;
  u8_t blue_field_position;
  u8_t rsvd_mask_size;
  u8_t rsvd_field_position;
  u8_t direct_color_mode_info;

  /* Mandory information for VBE 2.0 and above.  */
  u32_t phys_base_addr;
  u32_t reserved2;
  u16_t reserved3;

  /* Mandory information for VBE 3.0 and above.  */
  u16_t lin_bytes_per_scan_line;
  u8_t bnk_number_of_image_pages;
  u8_t lin_number_of_image_pages;
  u8_t lin_red_mask_size;
  u8_t lin_red_field_position;
  u8_t lin_green_mask_size;
  u8_t lin_green_field_position;
  u8_t lin_blue_mask_size;
  u8_t lin_blue_field_position;
  u8_t lin_rsvd_mask_size;
  u8_t lin_rsvd_field_position;
  u32_t max_pixel_clock;

  /* Reserved field to make structure to be 256 bytes long, VESA BIOS 
     Extension 3.0 Specification says to reserve 189 bytes here but 
     that doesn't make structure to be 256 bytes. So additional one is 
     added here.  */
  u8_t reserved4[189 + 1];
} __attribute__ ((packed));

void vesa_test()
{
  //        u32_t code = load_shared("realmode");

  u32_t code = load_binary("vesa");

  /*  struct vbe_info_block *vbe = (vbe_info_block *)(0x40000);
     vbe->signature[0] = 'V';
     vbe->signature[1] = 'B';
     vbe->signature[2] = 'E';
     vbe->signature[3] = '2';
   */

  u32_t res = 0;
  u16_t mode = 0x4111;
  cli();
  asm volatile ("movl %1, %%eax \n"
		"movw %2, %%bx \n"
		"call *%%eax":"=a" (res):"m"(code), "m"(mode));
  sti();
  printk("\nres=0x%X\n", res);
  if (((res & 0xff) != 0x4f) || (res & 0x100)) {
    printk("\nError setting videomode!\n");
    return;
  }

  struct vbe_mode_info_block *vbe = (vbe_mode_info_block *) 0x40000;
  printk("\n[%X]\n", vbe->mode_attributes);
  if (!(vbe->mode_attributes & 0x1))
    printk("Mode NOT supported!\n");

  if (vbe->mode_attributes & 0x80)
    printk("LFB NOT supported!\n");
  //      printk("%c",*res);

  printk("\n[%X]\n", vbe->phys_base_addr);

  extern u32_t *mpagedir;
  int i;
  for (i = 0; i < (640 * 480 * 2 / 4096); i += 1)
    k_mount_page(0xf0000 + i, 0xf0000 + i, mpagedir, 0);

  u16_t *ptr = (u16_t *) 0xf0000000 + 2;
  *ptr = 0xffff;

  //  u32_t *ptr;
  //  for(ptr = (u32_t *)0xA0000; (u32_t)ptr < 0x0f0000; ptr++)
  //    *ptr = 0xf2f3002f;

  //  code = load_shared("libapp1.so");
}

void putpixel(u32_t offs, u16_t code)
{
  u16_t *ptr = (u16_t *) 0xf0000000 + offs;
  *ptr = code;
}

void bar(u32_t x, u32_t y, u32_t size, u16_t color)
{
  u32_t i, j;
  for (i = y; i < y + size; i++)
    for (j = x; j < x + size; j++)
      putpixel(j + i * 640, color);
}

void hide_bar(u32_t x, u32_t y, u32_t size)
{
  u32_t i, j;
  for (i = y; i < y + size; i++)
    for (j = x; j < x + size; j++)
      putpixel(j + i * 640, 0xffff);
}

static inline void EnableTimer()
{
  outportb(0x21, inportb(0x21) & 0xfe);	/* Enable timer */
}

asmlinkage void init()
{
  extern const string version;
  extern u32_t build;
  extern const string compile_date, compile_time;

  VGA *con = new VGA;
  TTY *tty1 = new TTY(80, 25);

  tty1->stdout = con;
  tty1->SetTextColor(WHITE);

  stdout = tty1;

  *stdout << "FOS OS. Revision " << version << ". Build #" << build << " " <<
      compile_date << " " << compile_time << "\n";

  printk
      ("--------------------------------------------------------------------------------");

#if 1
  ProcMan = new TProcMan;
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

  Tinterface *obj;
  obj = modules->access("tty");
  string elf_buf = new char[obj->info.size];
  obj->read(0, elf_buf, obj->info.size);

  ProcMan->exec(elf_buf);

  obj = modules->access("fs");
  elf_buf = new char[obj->info.size];
  obj->read(0, elf_buf, obj->info.size);
  ProcMan->exec(elf_buf);
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
  elf_test("tty");
  elf_test("app1");
  // elf_test("app2");
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

  printk
      ("\n--------------------------------------------------------------------------------");
  printk("All OK. System Halted.\n");
  printk("You can get new version at http://fos.codeworld.ru/");
  printk
      ("\n--------------------------------------------------------------------------------");

  while (1) {
    asm("incb 0xb8000+154\n" "movb $0x5e,0xb8000+155 ");

    pause();
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
