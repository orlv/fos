/*
 * Copyright (c) 2008 Sergey Gridassov
 */

#ifndef CONFIG_H
#define CONFIG_H
#include <sys/pci.h>
void parse_config();
typedef struct hndl {
	struct hndl *next;
	int vid;
	int did;
	char *point;
	char *exe;
} handlers_t;
void TryHandleDevice(int did, int vid, pci_addr_t *addr);
#endif
