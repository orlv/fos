/*
  Оригинальная версия
   Copyright (C) 2006 Oleg Fedorov
  Сервер RTC
   Copyright (C) 2008 Sergey Gridassov
*/
#include <sys/io.h>
#include <fos/message.h>
#include <fos/fos.h>
#include <fos/fs.h>
#include <stdlib.h>
#include <types.h>
#include <stdio.h>

struct time {
  u16_t year;
  u8_t month;
  u8_t day;
  u16_t hour;
  u8_t min;
  u8_t sec;
} __attribute__ ((packed));


static u32_t DecodeBCD(u32_t bcd) {
  u32_t dec = 0;
  u32_t mask = 0xf0000000;

  for (u32_t i = 0; i < 32; i += 4) {
    dec = 10 * dec + ((bcd & mask) >> (28 - i));
    mask >>= 4;
  }
  return dec;
}

static u8_t cmos_inb(u8_t addr) {
	outb(addr, 0x70);
	return inb(0x71);
}


size_t RTCRead(void *buf, size_t count) {
	if(count < sizeof(struct time))
		return 0;
	struct time *time = buf;
	time->year = DecodeBCD(cmos_inb(0x32)) * 100;
	time->year += DecodeBCD(cmos_inb(9));

	time->month = DecodeBCD(cmos_inb(8));
	time->day = DecodeBCD(cmos_inb(7));

	time->hour = DecodeBCD(cmos_inb(4));
	time->min = DecodeBCD(cmos_inb(2));
	time->sec = DecodeBCD(cmos_inb(0));

	return sizeof(struct time);
}

int main(int argc, char *argv[]) {
  printf("RTC started\n");
  resmgr_attach("/dev/rtc");
  struct message msg;
  struct time *buffer = malloc(sizeof(struct time));
  while(1) {
    msg.tid = 0;
	msg.recv_buf = buffer;
	msg.recv_size = sizeof(struct time);
	msg.flags = 0;
	receive(&msg);
	switch(msg.arg[0]) {
	case FS_CMD_ACCESS: 
		msg.arg[0] = 1;
		msg.arg[1] = sizeof(struct time);
		msg.arg[2] = NO_ERR;
		msg.send_size = 0;
		break;
	case FS_CMD_READ: {
		int readed = RTCRead(buffer, msg.send_size);
		if(readed < msg.send_size) 
			msg.arg[2] = ERR_EOF;
		else
			msg.arg[2] = NO_ERR;
		msg.arg[0] = readed;
		msg.send_size = msg.arg[0];
		msg.send_buf = buffer;
		break;
	}
	default:
	//	printf("rtc: unknown command %u %u %u %u\n", msg.arg[0], msg.arg[1], msg.arg[2], msg.arg[3]);
		msg.arg[0] = 0;
		msg.arg[2] = ERR_UNKNOWN_METHOD;
		msg.send_size = 0;
	}
	reply(&msg);
	}
	return 0;
}

