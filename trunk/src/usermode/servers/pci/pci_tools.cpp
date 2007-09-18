/*
	Copyright (C) 2007 Michael Zhilin
	FOS system
	PCI Configuration Server
 */

/**
 * Global headers
 */
#include <stdio.h>
#include <stddef.h>

/**
 * Local headers
 */
#include "pci_def.h"
#include "pci_types.h"
#include "pci_const.h"
#include "pci_private.h"

/**
 * Own header
 */
#include "pci_tools.h"

/**
 * Implementation
 */
const char* unknown_device = "Unknown device";

const char* get_name_by_class(uint class_id)
{
	const char* name = unknown_device;
	uint baseclass_id, subclass_id, interfaceclass_id;
	t_base_class_info_ptr* current_base_class = base_classes;
	t_dev_info_ptr* current_dev_info = NULL;
	t_dev_info* info;

	baseclass_id = (class_id >> 16) & 0x0FF;
	subclass_id  = (class_id >> 8) & 0x0FF;
	interfaceclass_id = class_id & 0xFF;

	CLASSES_SEARCH_VALUE_BY_KEY(current_base_class, baseclass_id)

	if(current_base_class->key == CLASSES_LAST_KEY) 
		return name;

	current_dev_info = current_base_class->value;
	CLASSES_SEARCH_VALUE_BY_KEY(current_dev_info, subclass_id);

	if(current_dev_info->key == CLASSES_LAST_KEY) 
		return name;

	info = &(current_dev_info->value);
	return info->name;
} 

void print_pci_conf(t_pci_item* item){
	
	uint offset,value;

	item->enable = TRUE;

	printf("PCI Conf for (%x,%x,%x)\n", item->bus_id, item->func_id, item->slot_id);

	for(offset = 0x0; offset < 0x40; offset+= 4){
		item->reg_id = offset;
		value = read_pci_conf(item,4);
		printf("\t%x\n", value);
	}
}
