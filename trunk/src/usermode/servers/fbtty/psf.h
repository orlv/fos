/*
  psf.h
  Copyright (C) 2007 Oleg Fedorov
*/

#ifndef _PSF_H
#define _PSF_H

#define PSF_MAGIC 0x0436

#define PSF_MODE_256_NOUNICODE 0 /* 256 characters, no unicode_data   */
#define PSF_MODE_512_NOUNICODE 1 /* 512 characters, no unicode_data   */
#define PSF_MODE_256_UNICODE   2 /* 256 characters, with unicode_data */
#define PSF_MODE_512_UNICODE   3 /* 512 characters, with unicode_data */

struct psf_header {
  u16_t psf_magic;
  u8_t  psf_mode;
  u8_t  psf_fontheight;
}__attribute__ ((packed));

#endif
