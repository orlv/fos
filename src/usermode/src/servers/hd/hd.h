/*
  drivers/block/hd/hd.h
  Copyright (C) 2005-2007 Oleg Fedorov
*/

#ifndef __HD_H
#define __HD_H

#include <types.h>

#define HD_SECTOR_SIZE 512

/* Hd controller regs. Ref: IBM AT Bios-listing */
#define HD_DATA         0x1f0	/* _CTL when writing */
#define HD_ERROR        0x1f1	/* see err-bits */
#define HD_NSECTOR      0x1f2	/* nr of sectors to read/write */
#define HD_SECTOR       0x1f3	/* starting sector */
#define HD_LCYL         0x1f4	/* starting cylinder */
#define HD_HCYL         0x1f5	/* high byte of starting cyl */
#define HD_CURRENT      0x1f6	/* 101dhhhh , d=drive, hhhh=head */
#define HD_STATUS       0x1f7	/* see status-bits */
#define HD_FEATURE      HD_ERROR	/* same io address, read=error, write=feature */
#define HD_PRECOMP      HD_FEATURE	/* obsolete use of this port - predates IDE */
#define HD_COMMAND      HD_STATUS	/* same io address, read=status, write=cmd */

struct hd_drive_id {
  u16_t config;			/* lots of obsolete bit flags */
  u16_t cyls;			/* Obsolete, "physical" cyls */
  u16_t reserved2;		/* reserved (word 2) */
  u16_t heads;			/* Obsolete, "physical" heads */
  u16_t track_bytes;		/* unformatted bytes per track */
  u16_t sector_bytes;		/* unformatted bytes per sector */
  u16_t sectors;		/* Obsolete, "physical" sectors per track */
  u16_t vendor0;		/* vendor unique */
  u16_t vendor1;		/* vendor unique */
  u16_t vendor2;		/* Retired vendor unique */
  u8_t serial_no[20];		/* 0 = not_specified */
  u16_t buf_type;		/* Retired */
  u16_t buf_size;		/* Retired, 512 byte increments
				 * 0 = not_specified
				 */
  u16_t ecc_bytes;		/* for r/w long cmds; 0 = not_specified */
  u8_t fw_rev[8];		/* 0 = not_specified */
  u8_t model[40];		/* 0 = not_specified */
  u8_t max_multsect;		/* 0=not_implemented */
  u8_t vendor3;			/* vendor unique */
  u16_t dword_io;		/* 0=not_implemented; 1=implemented */
  u8_t vendor4;			/* vendor unique */
  u8_t capability;		/* (upper byte of word 49)
				 *  3:  IORDYsup
				 *  2:  IORDYsw
				 *  1:  LBA
				 *  0:  DMA
				 */
  u16_t reserved50;		/* reserved (word 50) */
  u8_t vendor5;			/* Obsolete, vendor unique */
  u8_t tPIO;			/* Obsolete, 0=slow, 1=medium, 2=fast */
  u8_t vendor6;			/* Obsolete, vendor unique */
  u8_t tDMA;			/* Obsolete, 0=slow, 1=medium, 2=fast */
  u16_t field_valid;		/* (word 53)
				 *  2:  ultra_ok        word  88
				 *  1:  eide_ok         words 64-70
				 *  0:  cur_ok          words 54-58
				 */
  u16_t cur_cyls;		/* Obsolete, logical cylinders */
  u16_t cur_heads;		/* Obsolete, l heads */
  u16_t cur_sectors;		/* Obsolete, l sectors per track */
  u16_t cur_capacity0;		/* Obsolete, l total sectors on drive */
  u16_t cur_capacity1;		/* Obsolete, (2 words, misaligned int)     */
  u8_t multsect;		/* current multiple sector count */
  u8_t multsect_valid;		/* when (bit0==1) multsect is ok */
  u32_t lba_capacity;		/* Obsolete, total number of sectors */
  u16_t dma_1word;		/* Obsolete, single-word dma info */
  u16_t dma_mword;		/* multiple-word dma info */
  u16_t eide_pio_modes;		/* bits 0:mode3 1:mode4 */
  u16_t eide_dma_min;		/* min mword dma cycle time (ns) */
  u16_t eide_dma_time;		/* recommended mword dma cycle time (ns) */
  u16_t eide_pio;		/* min cycle time (ns), no IORDY  */
  u16_t eide_pio_iordy;		/* min cycle time (ns), with IORDY */
  u16_t words69_70[2];		/* reserved words 69-70
				 * future command overlap and queuing
				 */
  /* HDIO_GET_IDENTITY currently returns only words 0 through 70 */
  u16_t words71_74[4];		/* reserved words 71-74
				 * for IDENTIFY PACKET DEVICE command
				 */
  u16_t queue_depth;		/* (word 75)
				 * 15:5 reserved
				 *  4:0 Maximum queue depth -1
				 */
  u16_t words76_79[4];		/* reserved words 76-79 */
  u16_t major_rev_num;		/* (word 80) */
  u16_t minor_rev_num;		/* (word 81) */
  u16_t command_set_1;		/* (word 82) supported
				 * 15:  Obsolete
				 * 14:  NOP command
				 * 13:  READ_BUFFER
				 * 12:  WRITE_BUFFER
				 * 11:  Obsolete
				 * 10:  Host Protected Area
				 *  9:  DEVICE Reset
				 *  8:  SERVICE Interrupt
				 *  7:  Release Interrupt
				 *  6:  look-ahead
				 *  5:  write cache
				 *  4:  PACKET Command
				 *  3:  Power Management Feature Set
				 *  2:  Removable Feature Set
				 *  1:  Security Feature Set
				 *  0:  SMART Feature Set
				 */
  u16_t command_set_2;		/* (word 83)
				 * 15:  Shall be ZERO
				 * 14:  Shall be ONE
				 * 13:  FLUSH CACHE EXT
				 * 12:  FLUSH CACHE
				 * 11:  Device Configuration Overlay
				 * 10:  48-bit Address Feature Set
				 *  9:  Automatic Acoustic Management
				 *  8:  SET MAX security
				 *  7:  reserved 1407DT PARTIES
				 *  6:  SetF sub-command Power-Up
				 *  5:  Power-Up in Standby Feature Set
				 *  4:  Removable Media Notification
				 *  3:  APM Feature Set
				 *  2:  CFA Feature Set
				 *  1:  READ/WRITE DMA QUEUED
				 *  0:  Download MicroCode
				 */
  u16_t cfsse;			/* (word 84)
				 * cmd set-feature supported extensions
				 * 15:  Shall be ZERO
				 * 14:  Shall be ONE
				 * 13:6 reserved
				 *  5:  General Purpose Logging
				 *  4:  Streaming Feature Set
				 *  3:  Media Card Pass Through
				 *  2:  Media Serial Number Valid
				 *  1:  SMART selt-test supported
				 *  0:  SMART error logging
				 */
  u16_t cfs_enable_1;		/* (word 85)
				 * command set-feature enabled
				 * 15:  Obsolete
				 * 14:  NOP command
				 * 13:  READ_BUFFER
				 * 12:  WRITE_BUFFER
				 * 11:  Obsolete
				 * 10:  Host Protected Area
				 *  9:  DEVICE Reset
				 *  8:  SERVICE Interrupt
				 *  7:  Release Interrupt
				 *  6:  look-ahead
				 *  5:  write cache
				 *  4:  PACKET Command
				 *  3:  Power Management Feature Set
				 *  2:  Removable Feature Set
				 *  1:  Security Feature Set
				 *  0:  SMART Feature Set
				 */
  u16_t cfs_enable_2;		/* (word 86)
				 * command set-feature enabled
				 * 15:  Shall be ZERO
				 * 14:  Shall be ONE
				 * 13:  FLUSH CACHE EXT
				 * 12:  FLUSH CACHE
				 * 11:  Device Configuration Overlay
				 * 10:  48-bit Address Feature Set
				 *  9:  Automatic Acoustic Management
				 *  8:  SET MAX security
				 *  7:  reserved 1407DT PARTIES
				 *  6:  SetF sub-command Power-Up
				 *  5:  Power-Up in Standby Feature Set
				 *  4:  Removable Media Notification
				 *  3:  APM Feature Set
				 *  2:  CFA Feature Set
				 *  1:  READ/WRITE DMA QUEUED
				 *  0:  Download MicroCode
				 */
  u16_t csf_default;		/* (word 87)
				 * command set-feature default
				 * 15:  Shall be ZERO
				 * 14:  Shall be ONE
				 * 13:6 reserved
				 *  5:  General Purpose Logging enabled
				 *  4:  Valid CONFIGURE STREAM executed
				 *  3:  Media Card Pass Through enabled
				 *  2:  Media Serial Number Valid
				 *  1:  SMART selt-test supported
				 *  0:  SMART error logging
				 */
  u16_t dma_ultra;		/* (word 88) */
  u16_t trseuc;			/* time required for security erase */
  u16_t trsEuc;			/* time required for enhanced erase */
  u16_t CurAPMvalues;		/* current APM values */
  u16_t mprc;			/* master password revision code */
  u16_t hw_config;		/* hardware config (word 93)
				 * 15:  Shall be ZERO
				 * 14:  Shall be ONE
				 * 13:
				 * 12:
				 * 11:
				 * 10:
				 *  9:
				 *  8:
				 *  7:
				 *  6:
				 *  5:
				 *  4:
				 *  3:
				 *  2:
				 *  1:
				 *  0:  Shall be ONE
				 */
  u16_t acoustic;		/* (word 94)
				 * 15:8 Vendor's recommended value
				 *  7:0 current value
				 */
  u16_t msrqs;			/* min stream request size */
  u16_t sxfert;			/* stream transfer time */
  u16_t sal;			/* stream access latency */
  u32_t spg;			/* stream performance granularity */
  u32_t lba_capacity_2[2];	/* 48-bit total number of sectors */
  u16_t words104_125[22];	/* reserved words 104-125 */
  u16_t last_lun;		/* (word 126) */
  u16_t word127;		/* (word 127) Feature Set
				 * Removable Media Notification
				 * 15:2 reserved
				 *  1:0 00 = not supported
				 *      01 = supported
				 *      10 = reserved
				 *      11 = reserved
				 */
  u16_t dlf;			/* (word 128)
				 * device lock function
				 * 15:9 reserved
				 *  8   security level 1:max 0:high
				 *  7:6 reserved
				 *  5   enhanced erase
				 *  4   expire
				 *  3   frozen
				 *  2   locked
				 *  1   en/disabled
				 *  0   capability
				 */
  u16_t csfo;			/*  (word 129)
				 * current set features options
				 * 15:4 reserved
				 *  3:  auto reassign
				 *  2:  reverting
				 *  1:  read-look-ahead
				 *  0:  write cache
				 */
  u16_t words130_155[26];	/* reserved vendor words 130-155 */
  u16_t word156;		/* reserved vendor word 156 */
  u16_t words157_159[3];	/* reserved vendor words 157-159 */
  u16_t cfa_power;		/* (word 160) CFA Power Mode
				 * 15 word 160 supported
				 * 14 reserved
				 * 13
				 * 12
				 * 11:0
				 */
  u16_t words161_175[15];	/* Reserved for CFA */
  u16_t words176_205[30];	/* Current Media Serial Number */
  u16_t words206_254[49];	/* reserved words 206-254 */
  u16_t integrity_word;		/* (word 255)
				 * 15:8 Checksum
				 *  7:0 Signature
				 */
} __attribute__ ((packed));

class hd{
private:
  struct drive_geometry {
    u32_t heads;
    u32_t tracks;
    u32_t spt;			/* sectors per track */
  } geometry;

  u32_t current_block;
  u32_t blocks_cnt;

  struct hd_drive_id *drive_id;

  res_t get_info();
  void check_error();
  res_t busy();
  res_t ready();
  res_t seek(u32_t block);
  u32_t rw(u16_t block, void *buffer, u8_t read);
  u32_t read_chs(u16_t head, u16_t track, u16_t sector, void *buffer);

public:
  hd();

  size_t read(off_t offset, void *buf, size_t count);
  size_t write(off_t offset, const void *buf, size_t count);
};

#endif
