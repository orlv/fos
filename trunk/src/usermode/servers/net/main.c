#include <stdio.h>
#include <fos/fos.h>
#include <fos/fs.h>
#include <fos/message.h>
#include <stdlib.h>
#include <string.h>
#include <sys/pci.h>
#include "nettypes.h"
#include "ethernet.h"
#include "rtl8139.h"
#include "arp.h"
#include "ip.h"
#define RECV_BUF_SIZE 1000
net_callbacks_t cb = { 0 };
volatile dev_t *dev = NULL;
THREAD(poller) {
  while(1) {
    if(dev) {
      if(dev->device_poll((dev_t *)dev, 1)) {
        u16_t type = EthernetType(dev->packet);
        switch(type) {
        case ETHERNET_TYPE_ARP:
          arp_handle(dev->packet);
          break;
        case ETHERNET_TYPE_IP:
//          ip_handle(dev->packet);
          break;
        case 0:
          break;
        default:
          printf("Unable to handle packet type: %x\n", type);
        }
      }
    }
    sched_yield();
  }
}
void net_testing() {
  printf("Net testing\n");
/*  u32_t test_ips[10] = {
	ETHERNET_FORMAT_IP(192,168,1,1),
	ETHERNET_FORMAT_IP(192,168,1,2),
	ETHERNET_FORMAT_IP(192,168,1,1),

	ETHERNET_FORMAT_IP(192,168,1,2),
	ETHERNET_FORMAT_IP(192,168,1,1),
	ETHERNET_FORMAT_IP(192,168,1,1),

	ETHERNET_FORMAT_IP(192,168,1,1),
	ETHERNET_FORMAT_IP(192,168,1,2),
	ETHERNET_FORMAT_IP(192,168,1,1),

	ETHERNET_FORMAT_IP(192,168,1,1)
  };
  printf("ARP test started\n");
  for(int i = 0; i < 10; i++) {
    char mac[6];
    int res = arp_get_mac(test_ips[i], &mac);
    if(res < 0) {
      printf("[%u] ARP test failed\n", i);
    } else {
      printf("[%u] ARP test passed, mac: %02x:%02x:%02x:%02x:%02x:%02x\n", i,
	  mac[0] & 0xff , mac[1]  & 0xff , mac[2]  & 0xff, mac[3]  & 0xff, mac[4]  & 0xff, mac[5]  & 0xff);
    }
  }
*/
}
int main(int argc, char *argv[]) {
  printf("FOS networking server\n");
  resmgr_attach("/dev/net");
  char *buffer = malloc(RECV_BUF_SIZE);
  arp_clear();
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
      if(msg.arg[1] == 0x10EC && msg.arg[2] == 0x8139 && !dev) {
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
       } else {
          dev = new;
          printf("Card started up successfully\n");
          dev->packet = malloc(ETH_FRAME_LEN);
          dev->ip =      ETHERNET_FORMAT_IP(192,168,1,8);
          dev->mask =    ETHERNET_FORMAT_IP(255,255,255,0);
          dev->gateway = ETHERNET_FORMAT_IP(192,168,1,1);
          thread_create((off_t)poller);
          net_testing();
       }
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
 
