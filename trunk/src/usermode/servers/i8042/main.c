#include <fos/fos.h>
#include <fos/fs.h>
#include <fos/message.h>
#include <stdio.h>
#include <stdlib.h>
#include "keyboard.h"
#include "mouse.h"

void thread_handler()
{
  if (interrupt_attach(1) != RES_SUCCESS) {
    printf("keyboard: can't attach interrupt!\n");
    exit(1);
  }
  unmask_interrupt(1);
  if (interrupt_attach(12) != RES_SUCCESS) {
    printf("mouse: can't attach interrupt!\n");
    exit(1);
  }
  unmask_interrupt(12);
  struct message msg;

  while (1) {
    msg.recv_size = 0;
    msg.tid = 0;
    msg.flags = MSG_ASYNC;
    receive(&msg);
    if (msg.arg[0] == SIGNAL_IRQ) {
      if (msg.arg[1] == 1) {
        keyboard_ps2_interrupt();
      } else if (msg.arg[1] == 12) {
        mouse_ps2_interrupt();
      }
    } else
      printf("i8042: unknown signal received!\n");
  }
}

int main(int argc, char *argv[])
{
  keyboard_ps2_init();

  mouse_ps2_init();

  struct message msg;
  char *buffer = malloc(KB_CHARS_BUFF_SIZE);

  thread_create((off_t) & MouseHandlerThread);
  thread_create((off_t) & thread_handler);
  resmgr_attach("/dev/keyboard");
  while (1) {
    msg.tid = 0;
    msg.recv_buf = buffer;
    msg.recv_size = KB_CHARS_BUFF_SIZE;
    msg.flags = 0;
    receive(&msg);

    switch (msg.arg[0]) {
    case FS_CMD_ACCESS:
      msg.arg[0] = 1;
      msg.arg[1] = KB_CHARS_BUFF_SIZE;
      msg.arg[2] = NO_ERR;
      msg.send_size = 0;
      break;

    case FS_CMD_WRITE:
      msg.arg[0] = kb_write(0, buffer, msg.recv_size);
      msg.send_size = 0;
      if (msg.arg[0] < msg.recv_size)
	msg.arg[2] = ERR_EOF;
      else
	msg.arg[2] = NO_ERR;
      break;

    case FS_CMD_READ:
      msg.arg[0] = kb_read(0, buffer, msg.send_size);
      if (msg.arg[0] < msg.send_size) {
	msg.send_size = msg.arg[0];
	msg.arg[2] = ERR_EOF;
      } else
	msg.arg[2] = NO_ERR;

      msg.send_buf = buffer;
      break;

    default:
      msg.arg[0] = 0;
      msg.arg[2] = ERR_UNKNOWN_CMD;
      msg.send_size = 0;
    }

    reply(&msg);
  }
}
