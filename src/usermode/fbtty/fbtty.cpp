/*
  Copyright (C) 2007 Oleg Fedorov
 */

#include <fos.h>
#include <string.h>
#include <stdio.h>
#include <fs.h>
#include "fbterm.h"

#define FBTTY_CMD_SET_MODE (BASE_CMD_N + 0)
#define FBTTY_LOAD_FONT (BASE_CMD_N + 1)
#define FBTTY_PUT_CH (BASE_CMD_N + 2)

fbterm *fb=0;

#define RECV_BUF_SIZE 2048

asmlinkage int main()
{
  char *buffer = new char[RECV_BUF_SIZE];
  fb = new fbterm;

  struct message msg;
  //resmgr_attach("/dev/fbtty");
  if(resmgr_attach("/dev/tty") != RES_SUCCESS)
    return -1;

  while(!fb->set_videomode(0x4114));
  while(!fb->load_font("/mnt/modules/font.psf"));
  //outs("Welcome to FOS\n");

  size_t len = dmesg(buffer, RECV_BUF_SIZE);
  fb->write(0, buffer, len);

  
  while (1) {
    msg.tid = _MSG_SENDER_ANY;
    msg.recv_size = RECV_BUF_SIZE;
    msg.recv_buf = buffer;
    receive(&msg);
    switch(msg.a0){
    case FS_CMD_ACCESS:
      msg.a0 = 1;
      msg.a1 = RECV_BUF_SIZE;
      msg.a2 = NO_ERR;
      break;

    case FBTTY_CMD_SET_MODE:
      msg.a0 = fb->set_videomode(msg.a1);
      break;

    case FBTTY_LOAD_FONT:
      msg.a0 = fb->load_font(buffer);
      break;

      /*    case FBTTY_PUT_CH:
      fb->out_ch(msg.a1);
      msg.a0 = 1;
      break;*/

    case FS_CMD_WRITE:
      msg.a0 = fb->write(0 /*msg.a2*/, buffer, msg.recv_size);
      msg.a2 = NO_ERR;
      break;
      
    default:
      msg.a0 = 0;
      msg.a2 = ERR_UNKNOWN_CMD;
    }

    msg.send_size = 0;
    reply(&msg);
  }

  return 0;
}
