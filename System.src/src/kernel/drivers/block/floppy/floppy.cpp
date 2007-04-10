/*
	drivers/block/floppy/floppy.cpp
	Copyright (C) 2004-2006 Oleg Fedorov
*/

#include "floppy.h"
#include <io.h>
#include <stdio.h>
#include <mm.h>

void dma_xfer(u16_t channel, u32_t physaddr, u16_t length, u8_t read);

#define ENABLE_FLOPPY_IRQ()	outportb(0x21, inportb(0x21) & 0xbf  )

volatile u8_t done;
volatile u8_t motor;		/* Отображает состояние мотора 0-выключен, 1-включен */
volatile u16_t mtick;		/* Больше нуля - то уменьшается на 1, когда станет равным 0 - мотор выключается */
volatile u16_t tmout;		/* Просто таймер, его небходимо будет убрать после добавления такой функции в ядро */

floppy::floppy()
{
  u8_t i;

  /* floppy 3.5" */
  geometry.heads = DG144_HEADS;
  geometry.tracks = DG144_TRACKS;
  geometry.spt = DG144_SPT;

  current_block = 0;
  blocks_cnt = geometry.heads * geometry.tracks * geometry.spt;

  motor = 0;			/* Отображает состояние мотора 0-выключен, 1-включен */
  mtick = 0;			/* Больше нуля - то уменьшается на 1, когда станет равным 0 - мотор выключается */
  tmout = 0;			/* Просто таймер, его небходимо будет убрать после добавления такой функции в ядро */

  done = 0;			/* Устанавливается в 1 при приходе прерывания от флоппи */
  dchange = 0;
  for (i = 0; i < 7; i++)
    status[i] = 0;
  statsz = 0;
  sr0 = 0;
  fdc_track = 0xff;

  track_buf = kmalloc(512);
  //  printk("0x%X",track_buf);

  ENABLE_FLOPPY_IRQ();

  reset();			/* Сброс контроллера дисковода (Обязательно!) */

  printk("floppy drive: OK\n");
}

void floppy::sendbyte(u16_t count)
{
  volatile u8_t msr;
  u8_t tmo;

  for (tmo = 0; tmo < 128; tmo++) {
    msr = inportb(FDC_MSR);
    if ((msr & 0xc0) == 0x80) {
      outportb(FDC_DATA, count);
      return;
    }
    inportb(0x80);		/* задержка */
  }
}

u16_t floppy::getbyte()
{
  volatile u8_t msr;
  u8_t tmo;

  for (tmo = 0; tmo < 128; tmo++) {
    msr = inportb(FDC_MSR);
    if ((msr & 0xd0) == 0xd0)
      return inportb(FDC_DATA);
    inportb(0x80);
  }

  return 0;
}

/* ожидает, пока не завершится обработка команды */
u8_t floppy::waitfdc(u8_t sensei)
{
  tmout = 18;			/* таймаут 1 секунда */

  /* ожидаем прерывания (IRQ6), обозначающего конец обрабоки команды */
  //  printk("[wait..");
  while (!done && tmout) ;
  //  printk(" done]");

  /* узнаем результат */
  statsz = 0;
  while ((statsz < 7) && (inportb(FDC_MSR) & (1 << 4))) {
    status[statsz++] = getbyte();
  }

  if (sensei) {
    sendbyte(CMD_SENSEI);
    sr0 = getbyte();
    fdc_track = getbyte();
  }

  done = 0;

  if (!tmout) {
    if (inportb(FDC_DIR) & 0x80)
      dchange = 1;
    printk("Wait floppy irq error!\r\n");
    return 0;
  }
  return 1;
}

void floppy::block2hts(u16_t block, u16_t & head, u16_t & track, u16_t & sector)
{
  head = (block % (geometry.spt * geometry.heads)) / (geometry.spt);
  track = block / (geometry.spt * geometry.heads);
  sector = block % geometry.spt;
}

void floppy::reset()
{
  /* остановим мотор и откючим IRQ/DMA */
  outportb(FDC_DOR, 0);

  mtick = 0;
  motor = 0;

  /* установим скорость обмена данными (500K/s) */
  outportb(FDC_DRS, 0);

  /* включим прерывания */
  outportb(FDC_DOR, 0x0c);

  done = 1;

  waitfdc(1);
  //  printk("ok");

  /* укажем тайминги привода (говорят, их надо узнавать из BIOS) */
  sendbyte(CMD_SPECIFY);
  sendbyte(0xdf);		/* SRT = 3ms, HUT = 240ms */
  sendbyte(0x02);		/* HLT = 16ms, ND = 0 */

  seek_track(1);

  recalibrate();

  inportb(FDC_DIR);
  dchange = 0;
}

u8_t floppy::diskchange()
{
  return dchange;
}

void floppy::motoron()
{
  if (!motor) {
    mtick = 0xfff;
    outportb(FDC_DOR, 0x1c);

    tmout = 2;
    while (tmout) ;

    motor = 1;
  }
}

void floppy::motoroff()
{
  if (motor) {
    mtick = 36;
  }
}

/* Рекалибровка привода */
void floppy::recalibrate()
{
  /* включим мотор */
  motoron();

  sendbyte(CMD_RECAL);
  sendbyte(0);

  /* ждём завершения */
  waitfdc(1);

  /* выключаем мотор */
  motoroff();
}

/* поиск дорожки */
u8_t floppy::seek_track(u32_t track)
{
  if (fdc_track == track)	/* уже здесь? */
    return 1;

  motoron();

  sendbyte(CMD_SEEK);
  sendbyte(0);
  sendbyte(track);

  /* ждём окончания поиска */
  if (!waitfdc(1))
    return 0;			/* таймаут :( */

  tmout = 1;
  while (tmout) ;

  motoroff();
  /* проверяем */
  if ((sr0 != 0x20) || (fdc_track != track))
    return 0;
  else
    return 1;
}

size_t floppy::read(off_t offset, void *buf, size_t count)
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
    if (rw(current_block, buf, 1)) {
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

size_t floppy::write(off_t offset, const void *buf, size_t count)
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
    if (rw(current_block, (void *)buf, 0)) {
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

u32_t floppy::seek(u32_t block)
{
  if (block < blocks_cnt) {
    current_block = block;
    return 0;			/* OK */
  }
  return 1;			/* Error */
}

u8_t floppy::rw(u16_t block, void *buf, u8_t read)
{
  u16_t head, track, sector, tries, i;
  //u16_t tries, head, sector, i;

  /* преобразуем логический адрес в физический */
  block2hts(block, head, track, sector);
  //  printk("block %d = head[%d]:track[%02d]:sector[%02d]\r\n",block,head,track,sector);

  /* запусти мотор флоппи */
  motoron();

  if (!read && buf) {
    /* скопируем данные из буфера */
    for (i = 0; i < (512 / 4); i++) {
      ((u32_t *) track_buf)[i] = ((u32_t *) buf)[i];
    }
  }

  for (tries = 0; tries < 3; tries++) {
    /* проверим, не сменился ли диск */
    if ((inportb(FDC_DIR) & 0x80)) {
      printk("Floppy: detected disk change!\n");
      dchange = 1;
      seek_track(1);		/* очистим статус смены диска */
      recalibrate();
      motoroff();
      return 0;
    }

    /* передвинем головку к нужной дорожке */
    if (!seek_track(track)) {
      motoroff();
      printk("Floppy: seek error\r\n");
      return 0;
    }

    /* зададим скорость считывания (500K/s) */
    outportb(FDC_CCR, 0);

    /* отправим команду */
    if (read) {
      dma_xfer(2, (u32_t) track_buf, 512, 0);
      sendbyte(CMD_READ);
    } else {
      dma_xfer(2, (u32_t) track_buf, 512, 1);
      sendbyte(CMD_WRITE);
    }

    sendbyte(head << 2);
    sendbyte(track);
    sendbyte(head);
    sendbyte(sector);
    sendbyte(2);		/* 512 unsigned chars/sector */
    sendbyte(geometry.spt);
    if (geometry.spt == DG144_SPT)
      sendbyte(DG144_GAP3RW);	/* gap 3 size for 1.44M read/write */
    else
      sendbyte(DG168_GAP3RW);	/* gap 3 size for 1.68M read/write */
    sendbyte(0xff);		/* DTL = unused */

    /* ожидаем завершения команды */
    if (!waitfdc(0))
      return 0;			/* таймаут */

    if ((status[0] & 0xc0) == 0)
      break;			/* прочитали */

    recalibrate();		/* ошибка, попробуем снова */
  }

  /* остановим мотор */
  motoroff();

  if (read && buf) {
    /* скопируем данные в буфер */
    for (i = 0; i < (512 / 4); i++) {
      ((u32_t *) buf)[i] = ((u32_t *) track_buf)[i];
    }
  }

/*
  printk("status unsigned chars: ");
  for (i = 0;i < statsz;i++)
    printk("%02x ",status[i]);

  printk("\n");
*/
  return (tries != 3);		/* если равно, то ошибка */
}

/* 
   Обработчик прерывания от контроллера дисковода.
   Устанавливает done в 1 по приходу прерывания 
*/
asmlinkage void floppy_handler()
{
  done = 1;
  outportb(0x20, 0x20);
}
