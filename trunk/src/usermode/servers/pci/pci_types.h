#ifndef _PCI_TYPES_H
#define _PCI_TYPES_H 

#include "pci_def.h"

/**
 * Types definitions
 */

typedef unsigned int uint;

typedef struct{
	const char* name;
	uint id;
}t_dev_info;

typedef struct{
	uint bus_id;
	uint slot_id;
	uint func_id;
	uint reg_id;
	bool enable;
}t_pci_item;

typedef struct{
	uint device_id;
	uint vendor_id;
	uint class_id;
	uint command;
	uint header;
}t_pci_dev_info;

/**
 * Classes description
 */

MAP_ITEM_DEF(uint, t_dev_info, t_dev_info_ptr)
MAP_ITEM_DEF(uint, t_dev_info_ptr*, t_base_class_info_ptr)

/**
 * Devices
 */

MAP_ITEM_DEF(uint, t_pci_dev_info*, t_pci_device_by_dev_id)
MAP_ITEM_DEF(uint, t_pci_device_by_dev_id*, t_pci_device_by_dev_vnd_id)

#endif 
