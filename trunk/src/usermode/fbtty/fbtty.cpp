/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos.h>
#include <string.h>
#include <stdio.h>
#include <fs.h>

struct vbe_mode_info_block
{
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

#define VBESRV_CMD_SET_MODE (BASE_CMD_N + 0)
#define FBTTY_CMD_SET_MODE (BASE_CMD_N + 0)
#define FBTTY_LOAD_FONT (BASE_CMD_N + 1)

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

u16_t *lfb;
struct vbe_mode_info_block *vbeinfo;
//char cfont[256][FONT_WIDTH * FONT_HEIGHT];

//asm("sys_font:");
//asm(".incbin \"char.mt\"");

//#define FONT_WIDTH 8
//#define FONT_HEIGHT 8

u8_t font_width  = 0;
u8_t font_height = 0;
char *font_rawdata = 0;
//extern char sys_font[256][FONT_WIDTH * FONT_HEIGHT];
u16_t gattrib = 0xffff;
u16_t gbackgr = 0;

u32_t scr_width;
u32_t scr_height;

void putpixel (int x, int y, u32_t c)
{
  int off = x + y*vbeinfo->x_resolution;
  lfb[off] = c;
}

void put_char(int x, int y, unsigned char c)
{
  for(int i = 0; i < font_height; i++) {
    for(int j = 0; j < font_width ; j++) {
      if(font_rawdata[font_height*c + i] & (1<<j))
	putpixel(x + font_width - j, y + i, gattrib);
      /*else
	putpixel(x + font_width -j, y + i, gbackgr);*/
    }
  }
}

#define FONTBUF_SIZE 0x5000
#define CH_SPACE 0

u32_t _x;
u32_t _y;

#define BORDER 30

void bar(u32_t x, u32_t y, size_t x_size, size_t y_size, u16_t color)
{
  for(u32_t i=y; i<y+y_size; i++)
    for(u32_t j=x; j<x+x_size; j++)
      putpixel(j,i, color);
}


void out_ch(const unsigned char ch)
{
  switch (ch) {
  case '\n':
    _x = BORDER;
    _y += font_height + CH_SPACE;
    /*    if (_y+font_height+CH_SPACE > scr_height ) {
      //scroll_up();
      }*/

    /*  case '\r':
    _x = 0;
    break;*/

  default:
    if (ch >= 0x20){
      put_char(_x, _y, ch);
      _x += font_width + CH_SPACE;
      if(_x+font_width+CH_SPACE+BORDER > scr_width){
	_x = BORDER;
	_y += font_height+CH_SPACE;
	// scroll_up();
      }
    }
  }
}

void outs(char *str)
{
  while(*str){
    out_ch(*str);
    str++;
  }
}

asmlinkage int main()
{
  vbeinfo = new vbe_mode_info_block;
  char *fontpath = new char[MAX_PATH_LEN];
  char *fontbuf = new char[FONTBUF_SIZE];
  size_t lfb_size = 0;
  lfb = 0;
  u32_t mode = 0;
  int fd;
  _x = CH_SPACE + BORDER;
  _y = CH_SPACE + BORDER;
  struct message msg;
  struct message msg1;
  resmgr_attach("/dev/fbtty");

  while (1) {
    msg.tid = _MSG_SENDER_ANY;
    msg.recv_size = MAX_PATH_LEN;
    msg.recv_buf = fontpath;
    receive(&msg);
    switch(msg.a0){
    case FS_CMD_ACCESS:
      msg.a0 = 1;
      msg.a1 = 0;
      msg.a2 = NO_ERR;
      msg.send_size = 0;
      break;

    case FBTTY_CMD_SET_MODE:
      printf("fbtty: trying to setup videomode\n");
      fd = open("/dev/vbe", 0);
      if(fd != -1) {
	msg1.a0 = VBESRV_CMD_SET_MODE;
	msg1.a1 = msg.a1;
	msg1.send_size = 0;
	msg1.recv_size = sizeof(vbe_mode_info_block);
	msg1.recv_buf = vbeinfo;
	msg1.tid = ((fd_t)fd)->thread;
	send((message *)&msg1);
	printf("fbtty: result=0x%X, vbeinfo=0x%X\n", msg1.a0, vbeinfo);

	if(msg1.a0) { /* режим успешно установлен */
	  lfb_size = vbeinfo->x_resolution * vbeinfo->y_resolution *  vbeinfo->bits_per_pixel;
	  printf("lfbtty: lfb address=%X   \n"	\
		 "lfbtty: mode %dx%d@%dbpp \n"	\
		 "lfbtty: lfb size=0x%X    \n",
		 vbeinfo->phys_base_addr,
		 vbeinfo->x_resolution,
		 vbeinfo->y_resolution,
		 vbeinfo->bits_per_pixel,
		 lfb_size);
	  
	  scr_width = vbeinfo->x_resolution;
	  scr_height = vbeinfo->y_resolution;
	  
	  if(lfb)
	    kfree((off_t)lfb);
	  lfb = (u16_t *)kmemmap(vbeinfo->phys_base_addr, lfb_size);
	  mode = msg.a1;
	  msg.a0 = 1;
	  bar(10, 10, scr_width-20, scr_height-20, 0x4aad);
	} else {
	  msg.a0 = 0;
	}
	msg.send_size = 0;
	close(fd);
      }
      break;

    case FBTTY_LOAD_FONT:
      printf("fbtty: loading font %s\n", fontpath);
      fd = open(fontpath, 0);
      if(fd != -1) {
	read(fd, fontbuf, FONTBUF_SIZE);
	if(((psf_header *)fontbuf)->psf_magic == PSF_MAGIC) {
	  printf("psf format detected\n"	\
		 "psf_mode=%d\n"		\
		 "psf_fontheight=%d\n",
		 ((psf_header *)fontbuf)->psf_mode,
		 ((psf_header *)fontbuf)->psf_fontheight);
	}

	font_width = 8;
	font_height = ((psf_header *)fontbuf)->psf_fontheight;
	font_rawdata = (char *)((u32_t) fontbuf + sizeof(psf_header));
	msg.a0 = 1;
	msg.send_size = 0;
	close(fd);

	bar(10, 10, scr_width-20, scr_height-20, 0x4aad);
	
	outs("FOS - FOS is Operating system\n" \
	     "Testing font chars:\n");
	
	for(int i=0; i<256; i++)
	  out_ch(i);

	outs("\nIt's all.. :)\n");

      } else
	printf("fbtty: file not found\n");
      break;
      
    default:
      msg.a0 = 0;
      msg.a2 = ERR_UNKNOWN_CMD;
      msg.send_size = 0;
    }
    reply(&msg);
  }
  while(1);
  return 0;
}
