#include <stdio.h>
#include <fos/fos.h>
#include <stdlib.h>
#include <fos/message.h>
#include <string.h>
#include <fos/fs.h>
#include <sys/pci.h>
#include "rtl8139.h"
#include "uip.h"
#include "uip_arp.h"
#include "timer.h"

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

#define RECV_BUF_SIZE 1000
int connect_pci();
void uip_log(char *msg) {
	printf("UIP: %s\n", msg);
}
int main(int argc, char *argv[]) {
  uip_ipaddr_t ipaddr;
  int i;
  struct timer periodic_timer, arp_timer;
	if(connect_pci() < 0) return 1;
	printf("Starting up stack!\n");
	uip_init();
  uip_ipaddr(ipaddr, 192,168,1,8);
  uip_sethostaddr(ipaddr);
  uip_ipaddr(ipaddr, 192,168,1,1);
  uip_setdraddr(ipaddr);
  uip_ipaddr(ipaddr, 255,255,255,0);
  uip_setnetmask(ipaddr);
	uip_setethaddr(rtl8139_mac);
	dhcpc_init(rtl8139_mac, 6);
	resolv_init();
	httpd_init();
  while(1) {
    uip_len = rtl8139_poll();

    if(uip_len > 0) {
      if(BUF->type == htons(UIP_ETHTYPE_IP)) {
	uip_arp_ipin();
	uip_input();
	/* If the above function invocation resulted in data that
	   should be sent out on the network, the global variable
	   uip_len is set to a value > 0. */
	if(uip_len > 0) {
	  uip_arp_out();
	  rtl8139_transmit();
	}
      } else if(BUF->type == htons(UIP_ETHTYPE_ARP)) {
	uip_arp_arpin();
	/* If the above function invocation resulted in data that
	   should be sent out on the network, the global variable
	   uip_len is set to a value > 0. */
	if(uip_len > 0) {
	  rtl8139_transmit();
	}
      }

    } else if(timer_expired(&periodic_timer)) {
      timer_reset(&periodic_timer);
      for(i = 0; i < UIP_CONNS; i++) {
	uip_periodic(i);
	/* If the above function invocation resulted in data that
	   should be sent out on the network, the global variable
	   uip_len is set to a value > 0. */
	if(uip_len > 0) {
	  uip_arp_out();
	  rtl8139_transmit();
	}
      }

#if UIP_UDP
      for(i = 0; i < UIP_UDP_CONNS; i++) {
	uip_udp_periodic(i);
	/* If the above function invocation resulted in data that
	   should be sent out on the network, the global variable
	   uip_len is set to a value > 0. */
	if(uip_len > 0) {
	  uip_arp_out();
	  rtl8139_transmit();
	}
      }
#endif /* UIP_UDP */
      
      /* Call the ARP timer function every 10 seconds. */
      if(timer_expired(&arp_timer)) {
	timer_reset(&arp_timer);
	uip_arp_timer();
      }
    }
  }

	return 0;
}
 
void
dhcpc_configured(const struct dhcpc_state *s)
{
  uip_sethostaddr(s->ipaddr);
  uip_setnetmask(s->netmask);
  uip_setdraddr(s->default_router);
  resolv_conf(s->dnsaddr);
}

void route_appcalls() {
	resolv_appcall();
	dhcpc_appcall();
}

void
resolv_found(char *name, u16_t *ipaddr)
{
//  u16_t *ipaddr2;
  
  if(ipaddr == NULL) {
    printf("Host '%s' not found.\n", name);
  } else {
    printf("Found name '%s' = %d.%d.%d.%d\n", name,
	   htons(ipaddr[0]) >> 8,
	   htons(ipaddr[0]) & 0xff,
	   htons(ipaddr[1]) >> 8,
	   htons(ipaddr[1]) & 0xff);
    /*    webclient_get("www.sics.se", 80, "/~adam/uip");*/
  }
}

int connect_pci() {
  resmgr_attach("/dev/uip");
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
        if(rtl8139_init(addr) < 0) {
          printf("Failure!\n");
          free(addr);
          return -1;
       } else {
          printf("Card started up successfully\n");
	  return 0;
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
