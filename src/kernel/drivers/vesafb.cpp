/*
 * kernel/drivers/vesa.cpp
 * Copyright (C) 2007 Oleg Fedorov
 *
 * основано на коде из GRUB
 */

#include <fs.h>
#include <stdio.h>
#include <hal.h>

struct vbe_info_block
{
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

#define INJ_ADDRESS 0x500

/*void putpixel(u16_t *framebuffer, u32_t offs, u16_t code)
{
  //u16_t *ptr = (u16_t *) 0xf0000000+offs;
  freamebuffer[offset] = code;
  }*/

void vesafb_srv()
{
  while(1) {
    int fd = open("/mnt/modules/int16b", 0);
    if(fd != -1) {
      char *buf = (char *)INJ_ADDRESS; //new char[512];
      read(fd, buf, 512);
      close(fd);

      struct vbe_mode_info_block *vbeinfo;
      u16_t mode = 0x4117;
      int result = 0;
      hal->cli();
      asm volatile("call *%%eax":"=a"(result), "=b"(vbeinfo):"a"(buf), "c"(mode));
      hal->sti();
      printk("vesafb: result=0x%X, vbeinfo=0x%X\n", result, vbeinfo);
      if (((result & 0xff) != 0x4f) || (result & 0x100)) {
	printk("vesafb: error setting videomode 0x%X!\n", mode);
	while(1);
      }

      size_t lfb_size = vbeinfo->x_resolution * vbeinfo->y_resolution *  vbeinfo->bits_per_pixel;
      
      printk("vesafb: lfb address=%X   \n" \
	     "vesafb: mode %dx%d@%dbpp \n" \
	     "vesafb: lfb size=0x%X    \n",
	     vbeinfo->phys_base_addr,
	     vbeinfo->x_resolution,
	     vbeinfo->y_resolution,
	     vbeinfo->bits_per_pixel,
	     lfb_size);

      u16_t *lfb = (u16_t *)hal->kmem->mem_alloc_phys(vbeinfo->phys_base_addr, lfb_size);

      while(1) {
	for(u32_t j=0; j<1024; j++)
	  for(u32_t i=0; i<768; i++)
	    lfb[i*j] = ((i+1000000)/(j+1000));
      }

      while(1);
      break;
    } else
      continue;
  }

  while(1);
}
