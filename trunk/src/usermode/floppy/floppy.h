/*
  floppy/floppy.h
  Copyright (C) 2004-2007 Oleg Fedorov
*/

#ifndef __FLOPPY_H
#define __FLOPPY_H

#include <types.h>
//#include <tinterface.h>

#define FLOPPY_BUFF_SIZE 512
#define FLOPPY_IRQ_NUM     6

/* drive geometries */
#define DG144_HEADS       2	/* heads per drive (1.44M) */
#define DG144_TRACKS     80	/* number of tracks (1.44M) */
#define DG144_SPT        18	/* sectors per track (1.44M) */
#define DG144_GAP3FMT  0x54	/* gap3 while formatting (1.44M) */
#define DG144_GAP3RW   0x1b	/* gap3 while reading/writing (1.44M) */

#define DG168_HEADS       2	/* heads per drive (1.68M) */
#define DG168_TRACKS     80	/* number of tracks (1.68M) */
#define DG168_SPT        21	/* sectors per track (1.68M) */
#define DG168_GAP3FMT  0x0c	/* gap3 while formatting (1.68M) */
#define DG168_GAP3RW   0x1c	/* gap3 while reading/writing (1.68M) */

/* IO ports */
#define FDC_DOR  (0x3f2)	/* Digital Output Register */
#define FDC_MSR  (0x3f4)	/* Main Status Register (input) */
#define FDC_DRS  (0x3f4)	/* Data Rate Select Register (output) */
#define FDC_DATA (0x3f5)	/* Data Register */
#define FDC_DIR  (0x3f7)	/* Digital Input Register (input) */
#define FDC_CCR  (0x3f7)	/* Configuration Control Register (output) */

/* command unsigned chars (these are 765 commands + options such as MFM, etc) */
#define CMD_SPECIFY (0x03)	/* specify drive timings */
#define CMD_WRITE   (0xc5)	/* write data (+ MT,MFM) */
#define CMD_READ    (0xe6)	/* read data (+ MT,MFM,SK) */
#define CMD_RECAL   (0x07)	/* recalibrate */
#define CMD_SENSEI  (0x08)	/* sense interrupt status */
#define CMD_FORMAT  (0x4d)	/* format track (+ MFM) */
#define CMD_SEEK    (0x0f)	/* seek track */
#define CMD_VERSION (0x10)	/* FDC version */

class Floppy {
private:
  struct drive_geometry {
    u8_t heads;
    u8_t tracks;
    u8_t spt;			/* sectors per track */
  } geometry;

  u16_t current_block;
  u16_t blocks_cnt;

  //bool motor_on;       /* Отображает состояние мотора 0-выключен, 1-включен*/
  //u32_t motor_ticks;   /* Больше нуля - то уменьшается на 1, когда станет равным 0 - мотор выключается */
  //volatile bool *irq_done;
  
  u8_t dchange;
  u8_t status[7];
  u8_t statsz;

  u8_t sr0;
  u16_t fdc_track;

  void *track_buf;       /* адрес буфера дисковода  */
  void *track_buf_phys;  /* физический адрес буфера */

  void reset();
  void sendbyte(u8_t count);
  u8_t getbyte();
  u8_t waitfdc(u8_t sensei);
  void block2hts(u16_t block, u16_t & head, u16_t & track, u16_t & sector);
  u8_t diskchange();
  void motoron();
  void motoroff();
  void recalibrate();
  u8_t seek_track(u32_t _track);
  u32_t seek(u32_t block);
  u8_t rw(u16_t block, void *buf, u8_t read);
  
public:
  Floppy();

  size_t read(off_t offset, void *buf, size_t count);
  size_t write(off_t offset, const void *buf, size_t count);
};

#endif
