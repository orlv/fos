#include <stdio.h>
#include <fos/fos.h>
#include <fos/fs.h>
#include <fos/message.h>
#include <stdlib.h>
#include <string.h>
#include <sys/pci.h>
#include "nettypes.h"
#include "rtl8139.h"
#define RECV_BUF_SIZE 1000
net_callbacks_t cb = { 0 };
int main(int argc, char *argv[]) {
  printf("FOS networking server\n");
  resmgr_attach("/dev/net");
  char *buffer = malloc(RECV_BUF_SIZE);
  struct message msg;
  while (1) {
    msg.tid = _MSG_SENDER_ANY;
    msg.recv_size = RECV_BUF_SIZE;
    msg.recv_buf = buffer;
    receive(&msg);
    switch (msg.arg[0]) {
    case FS_CMD_ACCESS:
      msg.arg[0] = 1;
      msg.arg[1] = RECV_BUF_SIZE;
      msg.arg[2] = NO_ERR;
      break;

    case PCI_NOTIFY_NEW: {
      if(msg.arg[1] == 0x10EC && msg.arg[2] == 0x8139) {
        printf("Realtek 8139 starting up\n");
        pci_addr_t *addr = malloc(sizeof(pci_addr_t));
        memcpy(addr, buffer, sizeof(pci_addr_t));
        dev_t *new = malloc(sizeof(dev_t));
	new->addr = addr;
        new->device_init = rtl8139_init;
        if((new->device_init)(&cb, new) < 0) {
          printf("Failure!\n");
          free(addr);
          free(new);
       } else 
          printf("Card started up successfully\n");
      }
      break;
    }
    default:
      msg.arg[0] = 0;
      msg.arg[2] = ERR_UNKNOWN_CMD;
    }

    msg.send_size = 0;
    reply(&msg);
  }
}
 
