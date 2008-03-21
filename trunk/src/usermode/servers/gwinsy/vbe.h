/*
 *         FOS Graphics System
 * Copyright (c) 2007 Sergey Gridassov
 */
#ifndef VBE_H
#define VBE_H
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

#endif
