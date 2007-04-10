/*
	drivers/block/hd/hd.cpp
	Copyright (C) 2005-2006 Oleg Fedorov
*/

#include "hd.h"
#include <io.h>
#include <stdio.h>
#include <system.h>
#include <drivers/char/timer/timer.h>

hd::hd()
{
  if (get_info() != RES_SUCCESS)
    panic("Kernel panic: hda initialization error");

  printk("\nhda: \n");
  printk("Serial number - %s\n", drive_id->serial_no);
  printk("Model - %s\n", drive_id->model);
  printk("Cyl - %d\n", drive_id->cur_cyls);
  printk("Head - %d\n", drive_id->cur_heads);
  printk("Sec - %d\n", drive_id->cur_sectors);
  printk("lba_capacity - %d\n", drive_id->lba_capacity);

  geometry.heads = drive_id->cur_heads;
  geometry.tracks = drive_id->cur_cyls;
  geometry.spt = drive_id->cur_sectors;
  blocks_cnt = drive_id->cur_heads * drive_id->cur_cyls * drive_id->cur_sectors;
  current_block = 0;

  printk("hda: OK\n");
}

res_t hd::ready()
{
#warning FIX: изменить интервал времени для uptime()   (слишком велик)
  u32_t timeout = uptime() + 2;
  while (!(inportb(HD_STATUS) & 0x40)) {
    if (timeout < uptime()) {
      printk("HD_Timeout: ready\n");
      return RES_FAULT;
    }
  }
  return RES_SUCCESS;
}

res_t hd::busy()
{
  u32_t timeout = uptime() + 2;
  while (inportb(HD_STATUS) & 0x80) {
    if (timeout < uptime()) {
      printk("HD_Timeout: busy\n");
      return RES_FAULT2;
    }
  }
  return RES_SUCCESS;
}

void hd::check_error()
{
  if (inportb(HD_STATUS) & 0x1)
    panic("HD_STATUS: ERROR!!!!");
}

res_t hd::get_info()
{
  u32_t i, errno;
  drive_id = new struct hd_drive_id;

  if ((errno = busy()) != RES_SUCCESS)
    return errno;

  outportb(HD_CURRENT, 0xa0);
  if ((errno = ready()) != RES_SUCCESS)
    return errno;

  outportb(HD_STATUS, 0xec);	/* идентификация */
  if ((errno = busy()) != RES_SUCCESS)
    return errno;

  for (i = 0; (i < 256) && (inportb(HD_STATUS) & 0x08); i++) {
    if ((errno = busy()) != RES_SUCCESS)
      return errno;
    check_error();
    ((u16_t *) drive_id)[i] = inportw(HD_DATA);
    if ((i >= 10 && i <= 19) || (i >= 27 && i <= 46)) {
    asm("xchgb %%ah, %%al":"=a"(((u16_t *) drive_id)[i])
    :	  "a"(((u16_t *) drive_id)[i]));
    }
  }
  return RES_SUCCESS;
}

size_t hd::read(off_t offset, void *buf, size_t count)
{
  u32_t size = 0;
  if (!count)
    return 0;
  count = (count - 1) / 512 + 1;

  if ((offset / 512 + 1) > blocks_cnt)
    return 0;
  current_block = offset / 512 + 1;

  if ((count + current_block) > blocks_cnt)
    return 0;

  while (count) {
    if (rw(current_block, buf, 1) == 512) {
      size += 512;
      if (seek(current_block + 1))
	break;
      count--;
      buf = (void *)((u32_t) buf + 512);
    } else
      break;
  }
  return (size);
}

size_t hd::write(off_t offset, const void *buf, size_t count)
{
  u32_t size = 0;
  if (!count)
    return 0;
  count = (count - 1) / 512 + 1;

  if ((offset / 512 + 1) > blocks_cnt)
    return 0;
  current_block = offset / 512 + 1;

  if ((count + current_block) > blocks_cnt)
    return 0;

  while (count) {
    if (rw(current_block, (void *)buf, 0) == 512) {
      size += 512;
      if (seek(current_block + 1))
	break;
      count--;
      buf = (void *)((u32_t) buf + 512);
    } else
      break;
  }
  return (size);
}

res_t hd::seek(u32_t block)
{
  if (block < blocks_cnt) {
    current_block = block;
    return RES_SUCCESS;
  }
  return RES_FAULT;
}

u32_t hd::read_chs(u16_t head, u16_t track, u16_t sector, void *buffer)
{
  //  printk("%d,%d,%d,0x%X\n",head,track,sector,buffer);
  u32_t i;
  u32_t errno;
  if ((errno = busy()) != RES_SUCCESS)
    return errno;
  outportb(HD_CURRENT, 0xa0 | head);
  if ((errno = ready()) != RES_SUCCESS)
    return errno;
  outportb(HD_NSECTOR, 1);
  outportb(HD_SECTOR, sector);
  outportb(HD_LCYL, track);
  outportb(HD_HCYL, track >> 8);
  outportb(HD_STATUS, 0x20);

  for (i = 0; (i < 256) && (inportb(HD_STATUS) & 0x08); i++) {
    if ((errno = busy()) != RES_SUCCESS)
      return errno;
    check_error();
    ((u16_t *) buffer)[i] = inportw(HD_DATA);
  }

  return (i * 2);
}

u32_t hd::rw(u16_t block, void *buffer, u8_t read)
{
  if (read == 1) {
    return read_chs((current_block % (geometry.heads * geometry.spt)) /
		    geometry.spt,
		    current_block / (geometry.heads * geometry.spt),
		    current_block % geometry.spt, buffer);
  } else
    return 0;
}
