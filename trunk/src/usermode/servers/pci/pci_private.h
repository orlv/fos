#ifndef _PCI_PRIVATE_H
#define _PCI_PRIVATE_H 

#include "pci_types.h"

/**
 * Declaration
 */
uint get_pci_cmd_by_item(t_pci_item* item);
uint get_pci_data_port_by_item(t_pci_item* item);
uint write_pci_conf(t_pci_item* item, int cnt, uint value);
uint read_pci_conf(t_pci_item* item, int cnt); 

#endif
