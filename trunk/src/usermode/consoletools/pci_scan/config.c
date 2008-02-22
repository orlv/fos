/*
 * Copyright (c) 2008 Sergey Gridassov
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/pci.h>
#include <fos/fos.h>
#include <fos/message.h>
#include "config.h"

handlers_t *head = NULL;

static void ParseLine(char *line)
{
  if(!strlen(line)) return;

  char *original = line;
  char *tokens[4];
  int i = 0;

  for (char *ptr = strsep(&line, ":"); ptr && i < 4; ptr = strsep(&line, ":"), i++)
    tokens[i] = ptr;
  if (i < 4) {
    free(original);
    return;
  }
  int did = strtoul(tokens[0], NULL, 0);
  int vid = strtoul(tokens[1], NULL, 0);
  handlers_t *new = malloc(sizeof(handlers_t));
  new->vid = vid;
  new->did = did;
  new->point = tokens[2];
  new->exe = tokens[3];
  new->next = head;
  head = new;
}

void parse_config(void) {
  int hndl = open("/etc/pci", 0);
  if(!hndl) {
	printf("pci: Fatal error, config file not found .\n");
	exit(1);
  }
  int size = lseek(hndl, 0, SEEK_END);
  lseek(hndl, 0, SEEK_SET);
  char *config = malloc(size);

  read(hndl, config, size);
  close(hndl);

  for (char *ptr = strsep(&config, "\n"); ptr; ptr = strsep(&config, "\n")) {
    if (ptr[0] == '#')
      continue;			// комментарии
    ParseLine(ptr);
  }
}

void TryHandleDevice(int did, int vid, pci_addr_t *addr) {
	for(handlers_t *ptr = head; ptr; ptr = ptr->next) {
		if(ptr->did == did && ptr->vid == vid) {
			int handle = open(ptr->point, 0);
			int timeout = 0;
			int start = uptime();
			if(handle == -1) {
				printf("PCI: %s down, starting\n", ptr->point);
				if(!exec(ptr->exe, NULL)) {
					printf("PCI: %s not exists.\n", ptr->exe);
					return;
				}
				do {
					timeout = (uptime() - start) > 2000;
					handle = open(ptr->point, 0);
				} while(handle == -1 && !timeout);
				if(timeout) {
					printf("PCI: %s timeout (2 sec)\n", ptr->point);
					return;
				}
			}
			struct message msg;
			msg.flags = 0;
			msg.send_size = sizeof(pci_addr_t);
			msg.send_buf = addr;
			msg.tid = ((fd_t) handle)->thread;
			msg.arg[0] = PCI_NOTIFY_NEW;
			msg.arg[1] = vid;
			msg.arg[2] = did;
			send(&msg);
			close(handle);
			printf("PCI: device configured\n");
		}
	}
}
