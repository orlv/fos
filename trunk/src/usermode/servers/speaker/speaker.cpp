/*
  Copyright (C) 2007 Serge 'knott'
 */

#include <fos/fs.h>
#include <fos/fos.h>
#include <fos/message.h>
#include <fos/page.h>
#include <sys/io.h>
#include <stdio.h>

#define PIT_FREQ 0x1234DD
#define PIT_CONTROL 0x43
#define SPEAKER_PORT 0x61
#define PIT_CHANNEL_2 0x42
#define PCS_BEEP (BASE_CMD_N + 0)

void sound(int freq)
{
  int counter = PIT_FREQ / freq;

  outb(PIT_CONTROL, 0xB6);
  outb(PIT_CHANNEL_2, counter & 0x0000FFFF);
  outb(PIT_CHANNEL_2, counter >> 16);

  outb(SPEAKER_PORT, inb(SPEAKER_PORT) | 3);
}

void nosound(void)
{
  outb(SPEAKER_PORT, inb(SPEAKER_PORT | 0xFC));
}

void delay(int time)
{
  struct message msg;

  msg.tid = _MSG_SENDER_SIGNAL;
  msg.recv_size = 0;

  alarm(time);

  receive(&msg);
}

void beep(int freq, int d)
{
  sound(freq);

  delay(d);

  nosound();
}

int main()
{
  printf("PCSpeaker usermode driver\n");

  resmgr_attach("/dev/speaker");

  struct message msg;
  char *buf = new char[255];

  while (1) {
    msg.tid = _MSG_SENDER_ANY;
    msg.recv_size = 255;
    msg.recv_buf = buf;

    receive(&msg);

    switch (msg.arg[0]) {
    case PCS_BEEP:
      beep(msg.arg[1], msg.arg[2]);
      break;

    default:
      msg.arg[0] = 0;
      msg.arg[2] = ERR_UNKNOWN_CMD;
      break;
    }

    msg.send_size = 0;
    reply(&msg);
  }
}
