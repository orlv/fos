/*
  Copyright (C) 2007 Oleg Fedorov
*/

#include "fbterm.h"
#include <string.h>
#include <fos.h>
#include <fs.h>
#include "vbe.h"
#include "psf.h"

#define FONTBUF_SIZE 0x5000
#define CH_SPACE 0
#define X_BORDER 30
#define Y_BORDER 18

#define VBESRV_CMD_SET_MODE (BASE_CMD_N + 0)

fbterm::fbterm()
{
  fcolor = 0xffff;
  vbeinfo = new vbe_mode_info_block;
  _x = CH_SPACE + X_BORDER;
  _y = CH_SPACE + Y_BORDER;
}

void fbterm::putpixel (off_t x, off_t y, u32_t pixel)
{
  off_t off = x + y*vbeinfo->x_resolution;
  lfb_cache[off] = pixel;
}

void fbterm::putpixel_direct(off_t x, off_t y, u32_t pixel)
{
  off_t off = x + y*vbeinfo->x_resolution;
  lfb[off] = pixel;
}


void fbterm::put_char(off_t x, off_t y, unsigned char ch)
{
  for(int i = 0; i < font_height; i++) {
    for(int j = 0; j < font_width ; j++) {
      if(font_rawdata[font_height*ch + i] & (1<<j))
	putpixel(x + font_width - j, y + i, fcolor);
      /*else
	putpixel(x + font_width -j, y + i, bgcolor);*/
    }
  }
}

void fbterm::put_char_direct(off_t x, off_t y, unsigned char ch)
{
  for(int i = 0; i < font_height; i++) {
    for(int j = 0; j < font_width ; j++) {
      if(font_rawdata[font_height*ch + i] & (1<<j))
	putpixel_direct(x + font_width - j, y + i, fcolor);
      /*else
	putpixel(x + font_width -j, y + i, bgcolor);*/
    }
  }
}

void fbterm::bar(off_t x, off_t y, size_t x_size, size_t y_size, u16_t color)
{
  for(size_t i=y; i<y+y_size; i++)
    for(size_t j=x; j<x+x_size; j++)
      putpixel(j,i, color);
}

void fbterm::sync()
{
  memcpy(lfb, lfb_cache, lfb_size);
}

void fbterm::redraw()
{
  bar(10, 10, scr_width-20, scr_height-20, 0x4aad);

  _x = X_BORDER;
  _y = Y_BORDER;

  for(size_t i=0; i<buf_top; i++)
    do_out_ch(chars_buf[i]);
}

void fbterm::strip_buf()
{
  size_t i;
  for(i=0; i<(scr_width - X_BORDER*2)/font_width; i++)
    if(chars_buf[i] == '\n') {
      i++;
      break;
    }
  memcpy(chars_buf, &chars_buf[i], buf_top-i);
  buf_top -= i;
}

void fbterm::put_ch(unsigned char ch)
{
  if(buf_top >= chars_max_cnt)
    strip_buf();

  chars_buf[buf_top] = ch;
  buf_top++;
}

void fbterm::do_out_ch(unsigned char ch)
{
  switch (ch) {
  case '\n':
    _x = X_BORDER;
    _y += font_height + CH_SPACE;
    if (_y+Y_BORDER+font_height+CH_SPACE > scr_height ) {
      _y -= font_height+CH_SPACE;
      strip_buf();
      redraw();
    }
    break;

  case 0x08: /* backspace */
    if(_x > X_BORDER) _x -= font_width;
    buf_top -= 2;
    for(size_t i=_y; i<_y+font_height; i++)
      for(size_t j=_x; j<_x+font_width; j++){
	putpixel(j, i, 0x4aad);
      }

    
  default:
    if (ch >= 0x20){
      put_char(_x, _y, ch);
      _x += font_width + CH_SPACE;
      if(_x+font_width+CH_SPACE+X_BORDER > scr_width){
	_x = X_BORDER;
	_y += font_height+CH_SPACE;
	if (_y+Y_BORDER+font_height+CH_SPACE > scr_height ) {
	  _y -= font_height+CH_SPACE;
	  strip_buf();
	  redraw();
	}
      }
    }
  }
}

void fbterm::show_cursor(off_t x, off_t y)
{
  put_char_direct(_x, _y, 219);
}

void fbterm::hide_cursor(off_t x, off_t y)
{
  //bar(_x, _y, font_width, font_height, 0x4aad);
  for(size_t i=y; i<y+font_height; i++)
    for(size_t j=x; j<x+font_width; j++)
      putpixel_direct(j, i, 0x4aad);
}

void fbterm::out_ch(unsigned char ch)
{
  put_ch(ch);
  //if(!stop_out)
  do_out_ch(ch);
}

size_t fbterm::write(off_t offset, const void *buf, size_t count)
{
  //hide_cursor(_x, _y);
  //stop_out = 1;
  //sync();
  for(size_t i = 0; i < count; i++){
    out_ch(((const char *)buf)[i]);
  }
  
  //redraw();
  sync();

  show_cursor(_x, _y);
  //  stop_out = 0;
  return count;
}

int fbterm::set_videomode(u16_t mode)
{
  //printf("fbtty: trying to setup videomode\n");
  message msg;
  int fd = open("/dev/vbe", 0);
  if(fd != -1) {
    msg.a0 = VBESRV_CMD_SET_MODE;
    msg.a1 = mode;
    msg.send_size = 0;
    msg.recv_size = sizeof(vbe_mode_info_block);
    msg.recv_buf = vbeinfo;
    msg.tid = ((fd_t)fd)->thread;
    send((message *)&msg);
    //printf("fbtty: result=0x%X, vbeinfo=0x%X\n", msg.a0, vbeinfo);

    if(msg.a0) { /* режим успешно установлен */
      lfb_size = vbeinfo->x_resolution * vbeinfo->y_resolution * vbeinfo->bits_per_pixel/8;
      /*printf("lfbtty: lfb address=%X   \n"	\
	     "lfbtty: mode %dx%d@%dbpp \n"	\
	     "lfbtty: lfb size=0x%X    \n",
	     vbeinfo->phys_base_addr,
	     vbeinfo->x_resolution,
	     vbeinfo->y_resolution,
	     vbeinfo->bits_per_pixel,
	     lfb_size);*/
	  
      scr_width = vbeinfo->x_resolution;
      scr_height = vbeinfo->y_resolution;
	  
      if(lfb)
	kfree((off_t)lfb);
      if(lfb_cache)
	kfree((off_t)lfb_cache);

      lfb = (u16_t *)kmemmap(vbeinfo->phys_base_addr, lfb_size);
      lfb_cache = (u16_t *)kmalloc(lfb_size, 0);
      mode = msg.a1;

      bar(10, 10, scr_width-20, scr_height-20, 0x4aad);
      close(fd);
      return 1;
    }
    close(fd);
  }
  return 0;
}

int fbterm::load_font(char *fontpath)
{
  //printf("fbtty: loading font %s\n", fontpath);
  int fd = open(fontpath, 0);
  if(fd != -1) {
    if(!fontbuf)
      fontbuf = new char[FONTBUF_SIZE];

    read(fd, fontbuf, FONTBUF_SIZE);
    if(((psf_header *)fontbuf)->psf_magic == PSF_MAGIC) {
      /*      printf("psf format detected\n"	\
	     "psf_mode=%d\n"			\
	     "psf_fontheight=%d\n",
	     ((psf_header *)fontbuf)->psf_mode,
	     ((psf_header *)fontbuf)->psf_fontheight);*/
    } else {
      close(fd);
      return 0;
    }
    
    font_width = 8;
    font_height = ((psf_header *)fontbuf)->psf_fontheight;
    font_rawdata = (char *)((u32_t) fontbuf + sizeof(psf_header));

    chars_max_cnt = ((scr_width - X_BORDER*2)/font_width) * ((scr_height - Y_BORDER*2)/font_height);
    if(chars_buf)
      delete chars_buf;
    chars_buf = new char[chars_max_cnt];
    buf_top = 0;
    close(fd);

    return 1;
  }

  //printf("fbtty: file not found\n");
  return 0;
}
