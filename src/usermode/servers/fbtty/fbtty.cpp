/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos/message.h>
#include <fos/fos.h>
#include <fos/fs.h>
#include <string.h>
#include <stdio.h>
#include "fbterm.h"
#include <sched.h>

#define FBTTY_CMD_SET_MODE (BASE_CMD_N + 0)
#define FBTTY_LOAD_FONT (BASE_CMD_N + 1)
#define FBTTY_PUT_CH (BASE_CMD_N + 2)

fbterm * volatile fb=0;

#define RECV_BUF_SIZE 2048

asmlinkage int main()
{
  char *buffer = new char[RECV_BUF_SIZE];
  fb = new fbterm;

  struct message msg;
  //resmgr_attach("/dev/fbtty");
  if(resmgr_attach("/dev/tty") != RES_SUCCESS)
    return -1;

	// было 0x4114 - 800x600x16
	// поставил 0x4117 - 1024x768x16
  while(!fb->set_videomode(0x4117));
  while(!fb->load_font("/mnt/modules/font.psf"));
  //outs("Welcome to FOS\n");

//  size_t len = dmesg(buffer, RECV_BUF_SIZE);
//  fb->write(0, buffer, len);

  while (1) {
    msg.tid = _MSG_SENDER_ANY;
    msg.flags = 0;
    msg.recv_size = RECV_BUF_SIZE;
    msg.recv_buf = buffer;
    receive(&msg);
    switch(msg.arg[0]){
    case FS_CMD_ACCESS:
      msg.arg[0] = 1;
      msg.arg[1] = RECV_BUF_SIZE;
      msg.arg[2] = NO_ERR;
      break;

    case FBTTY_CMD_SET_MODE:
      msg.arg[0] = fb->set_videomode(msg.arg[1]);
      break;

    case FBTTY_LOAD_FONT:
      msg.arg[0] = fb->load_font(buffer);
      break;

    case FS_CMD_WRITE:
      msg.arg[0] = fb->write(0 /*msg.arg[2]*/, buffer, msg.recv_size);
      msg.arg[2] = NO_ERR;
      break;
    case 0xfffe:
	msg.arg[2] = NO_ERR;
	fb->lock(msg.arg[1]);
	break;
    case 0xffff:
        msg.arg[2] = NO_ERR;
	msg.arg[0] = fb->get_info()->phys_base_addr;
	msg.arg[1] = fb->get_info()->x_resolution;
	msg.arg[3] = fb->get_info()->y_resolution;
	break;
    default:
      msg.arg[0] = 0;
      msg.arg[2] = ERR_UNKNOWN_CMD;
    }

    msg.send_size = 0;
    reply(&msg);
  }

  return 0;
}
