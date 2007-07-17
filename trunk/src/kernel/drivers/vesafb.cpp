/*
 * kernel/drivers/vesa.cpp
 * Copyright (C) 2007 Oleg Fedorov
 *
 * основано на коде из GRUB
 */

#include <fs.h>
#include <stdio.h>
#include <hal.h>

#define INJ_ADDRESS 0x500

#define VBESRV_CMD_SET_MODE (BASE_CMD_N + 0)

__attribute__((regparm(1))) struct vbe_mode_info_block * (*vbe_set_video_mode) (u16_t mode);

void vesafb_srv()
{
  char *realmod_inj = (char *)INJ_ADDRESS;
  vbe_set_video_mode = (__attribute__((regparm(1))) vbe_mode_info_block * (*)(u16_t)) INJ_ADDRESS;
  struct vbe_mode_info_block * vbeinfo;
  while(1) {
    int fd = open("/mnt/modules/int16b", 0);
    if(fd != -1) {
      read(fd, realmod_inj, 512);
      close(fd);
      printk("vesafb: ready\n");
      break;
    } else
      continue;
  }

  struct message msg;
  resmgr_attach("/dev/vbe");
 
  while (1) {
    msg.tid = _MSG_SENDER_ANY;
    msg.recv_size = 0;
    receive(&msg);

    switch(msg.a0){
    case FS_CMD_ACCESS:
      msg.a0 = 1;
      msg.a1 = 0;
      msg.a2 = NO_ERR;
      msg.send_size = 0;
      break;

    case VBESRV_CMD_SET_MODE:

      hal->mt_disable();
      hal->cli();
      hal->pic->lock(); /* обязательно необходимо запретить все IRQ */
      vbeinfo = vbe_set_video_mode(msg.a1);
      hal->pic->unlock();
      hal->sti();
      hal->mt_enable();

      if(vbeinfo) {
	msg.send_buf = vbeinfo;
	msg.a0 = 1;
	msg.send_size = 256; // sizeof(vbe_mode_info_block);
      } else {
	msg.a0 = 0;
	msg.send_size = 0;
      }

      break;

    default:
      msg.a0 = 0;
      msg.a2 = ERR_UNKNOWN_CMD;
      msg.send_size = 0;
    }
    reply(&msg);
  }

  while(1);
}
