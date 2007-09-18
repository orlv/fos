/*
	Copyright (C) 2007 Michael Zhilin
	FOS system
	PCI Configuration Server
 */

/**
 * Global headers
 */
#include <sys/io.h>
#include <stddef.h>
#include <errno.h>

/**
 * Local headers
 */
#include "pci_def.h"
#include "pci_types.h"
#include "pci_const.h"

/**
 * Own header
 */
#include "pci_private.h"

/**
 * Implementation
 */
uint get_pci_cmd_by_item(t_pci_item* item){
	
	if(item == NULL)
		return 0;

	if((item->bus_id <= PCI_BUS_ID_MAX)	&&
	   (item->slot_id <= PCI_SLOT_ID_MAX)	&&
	   (item->func_id <= PCI_FUNC_ID_MAX)	&&
	   (item->reg_id  <= PCI_REG_ID_MAX))
	{
		return 	(item->enable << 31)	|
			(item->bus_id << 16)	|
			(item->slot_id << 11)	|
			(item->func_id << 8)	|
			(item->reg_id & ~0x03);
	}

	return 0;
}

uint get_pci_data_port_by_item(t_pci_item* item){
	
	if(item != NULL)
		return (PCI_CONFIG_DAT + (item->reg_id & 0x03));

	return NULL;
}

uint write_pci_conf(t_pci_item* item, int cnt, uint value){
	uint data_port;
	uint cmd;

	if(cnt < 0 || cnt > 4 || cnt == 3) 
		return EINVAL; 

	cmd = get_pci_cmd_by_item(item);
	outl(cmd,PCI_CONFIG_CMD);

	if(cnt > 0)
	{
		data_port = get_pci_data_port_by_item(item);

		if(cnt == 1)
			outb(value,data_port);
		if(cnt == 2)
			outw(value,data_port);
		if(cnt == 4)
			outl(value,data_port);
	}
	return 0;
}

uint read_pci_conf(t_pci_item* item, int cnt){
	uint data = 0x0;
	uint data_port;
	uint cmd;

	if(cnt < 0 || cnt > 4 || cnt == 3) 
		return EINVAL; 

	cmd = get_pci_cmd_by_item(item);
	outl(cmd,PCI_CONFIG_CMD);

	if(cnt > 0)
	{
		data_port = get_pci_data_port_by_item(item);

		if(cnt == 1)
			data = inb(data_port);
		if(cnt == 2)
			data = inw(data_port);
		if(cnt == 4)
			data = inl(data_port);
	}

	return data;
} 
