#ifndef _PCI_H
#define _PCI_H 

/**
 * Declaration
 */
t_pci_dev_info* get_dev_info(t_pci_item* dev_item);
void get_pci_access();
void scan_pci(); 

#endif
