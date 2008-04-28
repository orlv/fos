/*
  floppy/floppy.cpp
  Copyright (C) 2004-2007 Oleg Fedorov
  
  - перенесено в usermode (Fri Jul 13 21:53:00 2007)
*/

#include <fos/fos.h>
#include <fos/fs.h>
#include <fos/message.h>
#include <fos/page.h>
#include <sys/io.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "floppy.h"
#include <sys/mman.h>
#include "dma.h"


#define BLOCK_SIZE 512

//volatile bool ready = 0;
Floppy *floppy;
volatile bool irq_done = 0;
volatile u32_t motor_ticks = 0; /* Больше нуля - то уменьшается на 1, когда станет равным 0 - мотор выключается */
volatile bool motor_on = 0;

THREAD(floppy_timer_thread)
{
  struct message msg;
  while(1) {
    msg.recv_size = 0;
    msg.tid = 0;
    msg.flags = MSG_ASYNC;
    alarm(1000);
    receive(&msg);

    if(msg.arg[0] == SIGNAL_IRQ) {
      if(msg.arg[1] == FLOPPY_IRQ_NUM){
        if (motor_ticks)
          motor_ticks--;
        else if (motor_on) {
          outb(0x0c, 0x3f2); /* выключим мотор флоппи */
          motor_on = 0;
        }
      }
    }// else
//      printf("floppy handler: unknown signal received! (0x%X)\n", msg.arg[0]);
  }
}

#define FLOPPY_XCHG_BUF_SIZE 0x1000

asmlinkage int main()
{
  printf("Usermode floppy driver\n");
  thread_create((off_t) &floppy_timer_thread, 0);

  if(interrupt_attach(FLOPPY_IRQ_NUM) == RES_SUCCESS)
    printf("floppy: interrupt attached\n");
  else {
    printf("floppy: can't attache interrupt!\n");
    exit(1);
  }
  
  floppy = new Floppy;

  struct message msg;
  char *buffer = new char[FLOPPY_XCHG_BUF_SIZE];

  resmgr_attach("/dev/fda");

  while (1) {
    msg.tid = 0;
    msg.recv_buf  = buffer;
    msg.recv_size = FLOPPY_XCHG_BUF_SIZE;
    msg.flags = 0;
    receive(&msg);

    if(msg.arg[0] == SIGNAL_IRQ){
      switch(msg.arg[1]){
      case FLOPPY_IRQ_NUM:
	unmask_interrupt(FLOPPY_IRQ_NUM);
	irq_done = 1;
	break;
      case 0:
	break;
    
      default:
	printf("floppy: unknown signal received! (%d)\n", msg.arg[0]);
      }
    } else {
      switch(msg.arg[0]){
      case FS_CMD_ACCESS:
	msg.arg[0] = 1;
	msg.arg[1] = FLOPPY_XCHG_BUF_SIZE;
	msg.arg[2] = NO_ERR;
	msg.send_size = 0;
	break;

      case FS_CMD_WRITE:
	msg.arg[0] = floppy->write(msg.arg[2], buffer, msg.recv_size);
	msg.send_size = 0;
	if(msg.arg[0] < msg.recv_size)
	  msg.arg[2] = ERR_EOF;
	else
	  msg.arg[2] = NO_ERR;
	break;

      case FS_CMD_READ:
	msg.arg[0] = floppy->read(msg.arg[2], buffer, msg.send_size);
	if(msg.arg[0] < msg.send_size) {
	  msg.send_size = msg.arg[0];
	  msg.arg[2] = ERR_EOF;
	} else
	  msg.arg[2] = NO_ERR;
	msg.send_buf = buffer;
	break;

      default:
	msg.arg[0] = 0;
	msg.arg[2] = ERR_UNKNOWN_METHOD;
	msg.send_size = 0;
      }
      reply(&msg);
   }
  }
  return 0;
}


void dma_xfer(u16_t channel, u32_t physaddr, u16_t length, u8_t read);

Floppy::Floppy()
{
  u8_t i;

  /* Floppy 3.5" */
  geometry.heads = DG144_HEADS;
  geometry.tracks = DG144_TRACKS;
  geometry.spt = DG144_SPT;

  current_block = 0;
  blocks_cnt = geometry.heads * geometry.tracks * geometry.spt;

  dchange = 0;
  for (i = 0; i < 7; i++)
    status[i] = 0;
  statsz = 0;
  sr0 = 0;
  fdc_track = 0xff;

//  track_buf_phys = kmalloc(FLOPPY_BUFF_SIZE, MEM_FLAG_LOWPAGE);
//  track_buf = kmemmap((offs_t)track_buf_phys, FLOPPY_BUFF_SIZE);
   track_buf = kmmap(0, FLOPPY_BUFF_SIZE, 0x40, 0);
   track_buf_phys = (void *)getpagephysaddr((off_t) track_buf);
  //  printf("0x%X",track_buf);

  unmask_interrupt(FLOPPY_IRQ_NUM);
  reset(); /* сброс контроллера дисковода */

  printf("floppy drive: OK\n");
}

void Floppy::sendbyte(u8_t count)
{
  volatile u8_t msr;

  for (u8_t tmo = 0; tmo < 128 || 1; tmo++) {
    msr = inb(FDC_MSR);
    if ((msr & 0xc0) == 0x80) {
      outb(count, FDC_DATA);
      return;
    }
//    inb(0x80);		/* задержка */
    sched_yield();
  }
}

u8_t Floppy::getbyte()
{
  volatile u8_t msr;

  for (u8_t tmo = 0; tmo < 128; tmo++) {
    msr = inb(FDC_MSR);
    if ((msr & 0xd0) == 0xd0)
      return inb(FDC_DATA);
//    inb(0x80);
    sched_yield();
  }

  return 0;
}

bool wait_irq_tmout(u32_t tmout)
{
  if(irq_done){
    irq_done = 0;
    return 0;
  }
  struct message msg;

  alarm(tmout);

  while (1) {
    msg.recv_size = 0;
    msg.flags = MSG_ASYNC;
    msg.tid = 0;
    receive(&msg);
    
    switch(msg.arg[1]){
    case FLOPPY_IRQ_NUM:
      unmask_interrupt(FLOPPY_IRQ_NUM);
      if(!alarm(0))
	break;
      else
	return 0;
    
    case 0:
      return 1;
    
    default:
      printf("floppy: unknown signal received! (%d)\n", msg.arg[0]);
    }
  }

  return 1;
}

void wait_tmout(u32_t tmout)
{
  struct message msg;
  alarm(tmout);
  while(1) {
    msg.recv_size = 0;
    msg.tid = 0;
    msg.flags = MSG_ASYNC;
    receive(&msg);

    switch(msg.arg[0]){
      case FLOPPY_IRQ_NUM:
	unmask_interrupt(FLOPPY_IRQ_NUM);
	irq_done = 1;
	break;

      case 0:
	return;
	
      default:
	printf("floppy: unknown signal received!\n");
    }
  }
}

/* ожидает, пока не завершится обработка команды */
u8_t Floppy::waitfdc(u8_t sensei)
{
  /* ожидаем прерывания (IRQ6), обозначающего конец обработки команды */
  //printf("[wait..");
  //sched_yield();
  bool tmout = wait_irq_tmout(1000);
  //printf(" done] ");

  /* узнаем результат */
  statsz = 0;
  while ((statsz < 7) && (inb(FDC_MSR) & (1 << 4))) {
    status[statsz++] = getbyte();
  }

  if (sensei) {
    sendbyte(CMD_SENSEI);
    sr0 = getbyte();
    fdc_track = getbyte();
  }

  if (tmout) {
    if (inb(FDC_DIR) & 0x80)
      dchange = 1;
    printf("floppy: irq wait timeout!\n");
    return 0;
  }
  return 1;
}

void Floppy::block2hts(u16_t block, u16_t & head, u16_t & track, u16_t & sector)
{
  head = (block % (geometry.spt * geometry.heads)) / (geometry.spt);
  track = block / (geometry.spt * geometry.heads);
  sector = block % geometry.spt;
}

void Floppy::reset()
{
  /* остановим мотор и откючим IRQ/DMA */
  outb(0, FDC_DOR);
  motor_ticks = 0;
  motor_on = 0;

  /* установим скорость обмена данными (500K/s) */
  outb(0, FDC_DRS);

  /* включим прерывания */
  outb(0x0c, FDC_DOR);

  waitfdc(1);

  /* укажем тайминги привода (говорят, их надо узнавать из BIOS) */
  sendbyte(CMD_SPECIFY);
  sendbyte(0xdf);		/* SRT = 3ms, HUT = 240ms */
  sendbyte(0x02);		/* HLT = 16ms, ND = 0 */

  seek_track(1);
  recalibrate();

  inb(FDC_DIR);
  dchange = 0;
}

u8_t Floppy::diskchange()
{
  return dchange;
}

void Floppy::motoron()
{
  if (!motor_on) {
    motor_ticks = 0xfff;
    outb(0x1c, FDC_DOR);
    wait_tmout(500);
    motor_on = 1;
  }
}

void Floppy::motoroff()
{
  if (motor_on) {
    motor_ticks = 2;
  }
}

/* Рекалибровка привода */
void Floppy::recalibrate()
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
u8_t Floppy::seek_track(u32_t track)
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

  wait_tmout(250);
  
  motoroff();
  /* проверяем */
  if ((sr0 != 0x20) || (fdc_track != track))
    return 0;
  else
    return 1;
}

size_t Floppy::read(off_t offset, void *buf, size_t count)
{
  size_t size = 0;

  if (!count)
    return 0;

  count = (count + BLOCK_SIZE - 1)/BLOCK_SIZE;
  offset = (offset + BLOCK_SIZE - 1)/BLOCK_SIZE;
  //printf("floppy: offset=0x%X, count=0x%X\n", offset, count);
  current_block = offset + 1;

  if (current_block > blocks_cnt)
    return 0;

  if ((current_block + count) > blocks_cnt)
    count = blocks_cnt - current_block;

  while (count) {
    if (rw(current_block, &((char *)buf)[size], 1)) {
      size += BLOCK_SIZE;
      if (seek(current_block + 1))
	break;
      count--;
    } else
      break;
  }
  return size;
}

size_t Floppy::write(off_t offset, const void *buf, size_t count)
{
  u32_t size = 0;

  if (!count)
    return 0;

  count = (count + BLOCK_SIZE - 1)/BLOCK_SIZE;
  offset = (offset + BLOCK_SIZE - 1)/BLOCK_SIZE;
  current_block = offset + 1;
  
  if (current_block > blocks_cnt)
    return 0;

  if ((current_block + count) > blocks_cnt)
    count = blocks_cnt - current_block;

  while (count) {
    if (rw(current_block, &((char *)buf)[size], 0)) {
      size += BLOCK_SIZE;
      if (seek(current_block + 1))
	break;
      count--;
    } else
      break;
  }
  return size;
}

u32_t Floppy::seek(u32_t block)
{
  if (block < blocks_cnt) {
    current_block = block;
    return 0;			/* OK */
  }
  return 1;			/* Error */
}

u8_t Floppy::rw(u16_t block, void *buf, u8_t read)
{
  u16_t head, track, sector, tries;

  /* преобразуем логический адрес в физический */
  block2hts(block, head, track, sector);
  //  printf("block %d = head[%d]:track[%02d]:sector[%02d]\r\n",block,head,track,sector);

  /* запусти мотор флоппи */
  motoron();

  if (!read && buf) /* скопируем данные из буфера */
    memcpy(track_buf, buf, BLOCK_SIZE);

  for (tries = 0; tries < 3; tries++) {
    /* проверим, не сменился ли диск */
    if ((inb(FDC_DIR) & 0x80)) {
      printf("floppy: detected disk change!\n");
      dchange = 1;
      seek_track(1);		/* очистим статус смены диска */
      recalibrate();
      motoroff();
      return 0;
    }

    /* передвинем головку к нужной дорожке */
    if (!seek_track(track)) {
      motoroff();
      printf("floppy: seek error\n");
      return 0;
    }

    /* зададим скорость считывания (500K/s) */
    outb(0, FDC_CCR);

    /* отправим команду */
    if (read) {
      dma_xfer(2, (u32_t) track_buf_phys, FLOPPY_BUFF_SIZE, 0);
      sendbyte(CMD_READ);
    } else {
      dma_xfer(2, (u32_t) track_buf_phys, FLOPPY_BUFF_SIZE, 1);
      sendbyte(CMD_WRITE);
    }

    sendbyte(head << 2);
    sendbyte(track);
    sendbyte(head);
    sendbyte(sector);
    sendbyte(2);		/* BLOCK_SIZE unsigned chars/sector */
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

  if (read && buf)
    memcpy(buf, track_buf, BLOCK_SIZE);
/*
  printf("status unsigned chars: ");
  for (i = 0;i < statsz;i++)
    printf("%02x ",status[i]);

  printf("\n");
*/
  return (tries != 3);		/* если равно, то ошибка */
}
