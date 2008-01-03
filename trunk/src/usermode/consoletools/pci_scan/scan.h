#ifndef SCAN_H
#define SCAN_H
#define	PCI_BUS_ID_MAX	255 	
#define	PCI_SLOT_ID_MAX	31
#define	PCI_FUNC_ID_MAX	7
#define	PCI_REG_ID_MAX	255

typedef struct  {
	int device_id;
	int vendor_id;
	int class_id;
} dev_info;

#define PCI_CONF_INVALID_DEVICE 0xFFFF
#define PCI_CONF_DEVICE_OFFSET 0x02
#define PCI_CONF_VENDOR_OFFSET 0x00
#define PCI_CONF_REVISION_OFFSET 0x08

#define BASE_CLASS_DEPRECATED	0x00
#define BASE_CLASS_STORAGE	0x01
#define BASE_CLASS_NETWORK	0x02
#define BASE_CLASS_DISPLAY	0x03
#define BASE_CLASS_MULTIMEDIA	0x04
#define BASE_CLASS_MEMORY	0x05
#define BASE_CLASS_BRIDGE	0x06
#define BASE_CLASS_COMMUN	0x07
#define BASE_CLASS_BASE_SYS_BUS	0x08
#define BASE_CLASS_INPUT_DEV	0x09
#define BASE_CLASS_DOCKING	0x0A
#define BASE_CLASS_PROCESSOR	0x0B
#define BASE_CLASS_SERIAL_BUS	0x0C


typedef struct {
	int base_class;
	int sub_class;
	char *text;
} pci_dev;

char *find_device(int class);
void scan_pci(int bus);
#endif
