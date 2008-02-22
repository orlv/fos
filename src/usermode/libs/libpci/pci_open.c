/*
 * Copyright (c) 2008 Sergey Gridassov
 */
#include <sys/pci.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int pci_open(pci_addr_t *addr) {
	char *buf = malloc(64);
	snprintf(buf, 64, "/dev/pci/%u/%u/%u/%u", addr->bus, addr->slot, addr->func, addr->enable);
	int ret = open(buf, 0);
	free(buf);
	return ret;
}
