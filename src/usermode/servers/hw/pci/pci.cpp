/*
	Copyright (C) 2007 Michael Zhilin
	FOS system
	PCI Configuration Server
 */

/**
 * Global headers
 */
#include <sys/io.h>
#include <stdio.h>

/**
 * Local headers
 */
#include "pci_def.h"
#include "pci_types.h"
#include "pci_const.h"
#include "pci_private.h"
#include "pci_tools.h"

/**
 * Own header
 */
#include "pci.h"

/**
 * Implementation
 */
t_pci_item* root_item;

void get_pci_access(){
	uint root_cmd, res;

	root_item = new t_pci_item;
	root_item->bus_id = 0;
	root_item->slot_id= 0;
	root_item->func_id= 0;
	root_item->reg_id= 0;
	root_item->enable= TRUE;

	res = inl(PCI_CONFIG_CMD);

	root_cmd = get_pci_cmd_by_item(root_item);
	outl(root_cmd,PCI_CONFIG_CMD);
	
	res = inl(PCI_CONFIG_DAT);
	
	if(res != 0xffffffff){
		scan_pci();
	}

	delete root_item;

	return;
}

void scan_pci(){
	t_pci_item* dev_item;
	uint dev_id;

	dev_item = new t_pci_item;

	dev_item->bus_id = 0x0;
	dev_item->func_id= 0x0;
	
	printf("Start PCI scanning (bus = %x, function = %x)...\n", dev_item->bus_id, dev_item->func_id);

	for(dev_id = 0; dev_id <= PCI_SLOT_ID_MAX; dev_id++){
		const char* class_name = NULL;
		t_pci_dev_info* dev_info;

		dev_item->slot_id = dev_id;

		dev_info = get_dev_info(dev_item);

		if(dev_info == NULL)
			continue;

		class_name = get_name_by_class(dev_info->class_id);

		printf("Device found: %x (%s) Vendor: %x Slot: %x\n",
			dev_info->device_id,
			class_name,
			dev_info->vendor_id,
			dev_id);

		if(((dev_info->class_id >> 16) & 0x0FF) == BASE_CLASS_BRIDGE){
			uint func_id;
			t_pci_dev_info* sub_dev_info;
			const char* sub_dev_class_name = NULL;

			for(func_id = 1; func_id <= PCI_FUNC_ID_MAX; func_id++)
			{
				dev_item->func_id = func_id;
				sub_dev_info = get_dev_info(dev_item);
				if(sub_dev_info == NULL)
					continue;

				sub_dev_class_name = get_name_by_class(sub_dev_info->class_id);

				printf("Device found: %x (%s) Vendor: %x Slot: %x Function: %x\n",
					sub_dev_info->device_id,
					sub_dev_class_name,
					sub_dev_info->vendor_id,
					dev_id,
					func_id);
				delete sub_dev_info;
			}

			dev_item->func_id = 0x00;
		}

		delete dev_info;
//		print_pci_conf(dev_item);
	}

	dev_item->enable= FALSE;
	dev_item->reg_id= 0;
	dev_item->slot_id = 0;

	read_pci_conf(dev_item,0);

	printf("Finished PCI scanning\n");
}


t_pci_dev_info* get_dev_info(t_pci_item* dev_item){
	uint device_id, class_id, vendor_id, command, tmp, intline;
	
	t_pci_dev_info* dev_info;

	dev_item->enable = TRUE;
	dev_item->reg_id = PCI_CONF_DEVICE_OFFSET;
	device_id = read_pci_conf(dev_item,2);

	if(device_id == 0x0 || device_id == PCI_CONF_INVALID_DEVICE)
		return NULL;

	dev_item->reg_id = PCI_CONF_VENDOR_OFFSET;
	vendor_id = read_pci_conf(dev_item,2);

	dev_item->reg_id = PCI_CONF_REVISION_OFFSET;
	class_id = (read_pci_conf(dev_item,4) & ~0xff) >> 8;

	dev_item->reg_id = PCI_CONF_COMMAND_OFFSET;
	command = read_pci_conf(dev_item,2);

	if(command & APPROP_BIT(PCIM_CMD_IO_SPACE) || command & APPROP_BIT(PCIM_CMD_MEMORY_SPACE) ){
		char i;
		for(i = 0; i < 6; i++){
			dev_item->reg_id = PCI_CONF_BAR(i);
			tmp = read_pci_conf(dev_item,4);
			if(tmp){
				if(PCIM_BAR_MEM(tmp)){
					const char* s_type = "unknown dimension";
					uint size,u_size,base_addr;

					if( ((tmp >> PCIM_BAR_MEM_TYPE_BIT) & 0x3) == PCIM_BAR_MEM_TYPE_32)
						s_type = "32 bit";

					if( ((tmp >> PCIM_BAR_MEM_TYPE_BIT) & 0x3) == PCIM_BAR_MEM_TYPE_64)
						s_type = "64 bit";

					write_pci_conf(dev_item,4,0xFFFFFFFF);
					u_size = read_pci_conf(dev_item,4);
					write_pci_conf(dev_item,4,tmp);

					size = PCIM_BAR_MEM_SIZE(u_size);
					base_addr = PCIM_BAR_MEM_32BASE(tmp);

					printf("BAR%i: memory %s %x-%x\n", i, s_type, base_addr, base_addr + size - 1);
				}

				if(PCIM_BAR_IO(tmp)){
					uint size,u_size,base_addr;

					write_pci_conf(dev_item,4,0xFFFFFFFF);
					u_size = read_pci_conf(dev_item,4);
					write_pci_conf(dev_item,4,tmp);

					size = PCIM_BAR_PORT_SIZE(u_size);
					base_addr = PCIM_BAR_PORT(tmp);

					printf("BAR%i: io port %x-%x\n", i, base_addr, base_addr + size - 1);
				}
			}
		}
	}

	dev_item->reg_id = PCI_CONF_INTLINE_OFFSET;
	intline = read_pci_conf(dev_item,2);

	dev_info = new t_pci_dev_info;	
	dev_info->device_id = device_id;
	dev_info->vendor_id = vendor_id;
	dev_info->class_id = class_id;
	dev_info->command = command;
	return dev_info;
}
