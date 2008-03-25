#include <fos/fos.h>
#include <fos/fs.h>
#include <fos/message.h>
#include <stdio.h>
#include <stdlib.h>
#include <fos/nsi.h>
#include "keyboard.h"
#include "mouse.h"

static char *buffer;

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

int access(struct message *msg)
{
  msg->arg[0] = 1;
  msg->arg[1] = KB_CHARS_BUFF_SIZE;
  msg->arg[2] = NO_ERR;
  msg->send_size = 0;
}

int write(struct message *msg)
{
  msg->arg[0] = kb_write(0, buffer, msg->recv_size);
  msg->send_size = 0;
  if (msg->arg[0] < msg->recv_size)
    msg->arg[2] = ERR_EOF;
  else
    msg->arg[2] = NO_ERR;
}

int read(struct message *msg)
{
  msg->arg[0] = kb_read(0, buffer, msg->send_size);
  if (msg->arg[0] < msg->send_size) {
    msg->send_size = msg->arg[0];
    msg->arg[2] = ERR_EOF;
  } else
    msg->arg[2] = NO_ERR;

  msg->send_buf = buffer;
}

int main(int argc, char *argv[])
{
  keyboard_ps2_init();
  mouse_ps2_init();

  buffer = (char *) malloc(KB_CHARS_BUFF_SIZE);

  thread_create((off_t) & MouseHandlerThread, 0);
  thread_create((off_t) & thread_handler, 0);
  nsi_t *interface = new nsi_t("/dev/keyboard");

  /* стандартные параметры */
  interface->std.recv_buf = buffer;
  interface->std.recv_size = KB_CHARS_BUFF_SIZE;

  /* объявим интерфейсы */
  interface->add(FS_CMD_ACCESS, &access);
  interface->add(FS_CMD_WRITE, &write);
  interface->add(FS_CMD_READ, &read);

  /* обрабатываем поступающие сообщения */
  while (1) {
    interface->wait_message();
  };
}
