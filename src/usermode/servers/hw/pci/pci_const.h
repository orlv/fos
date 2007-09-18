#ifndef _PCI_CONST_H
#define _PCI_CONST_H

#include "pci_types.h"
#include "pci_def.h"

#define TRUE 1
#define FALSE 0 

#define PCI_CONFIG_CMD 0x0cf8
#define PCI_CONFIG_DAT 0x0cfc

/* Максимальные значения 
 31 бит	- ENABLE/DISABLE 
 30-24	- RESERVED
 23-16	- BUS (8 бит)
 15-11	- Слот, он же девайс (5 байт)
 10-8	- Функция, она же поддевайс (3 байта)
 7-2	- Регистр (6 байт)
 2-0	- TBD 
*/

#define	PCI_BUS_ID_MAX	255 	
#define	PCI_SLOT_ID_MAX	31
#define	PCI_FUNC_ID_MAX	7
#define	PCI_REG_ID_MAX	255

/**
 * PCI Configuration Block ( Main Part )
 */
#define PCI_CONF_VENDOR_OFFSET 0x00
#define PCI_CONF_DEVICE_OFFSET 0x02
#define PCI_CONF_COMMAND_OFFSET 0x04
#define PCI_CONF_STATUS_OFFSET 0x06
#define PCI_CONF_REVISION_OFFSET 0x08
#define PCI_CONF_CLASS_OFFSET 0x09
// Detailed Class
#define PCI_CONF_INTERFACE_OFFSET 0x09
#define PCI_CONF_SUBCLASS_OFFSET 0x0A
#define PCI_CONF_BASECLASS_OFFSET 0x0B
#define PCI_CONF_BAR_OFFSET 0x10
#define PCI_CONF_BAR(x) (PCI_CONF_BAR_OFFSET + (x)*4)

#define PCI_CONF_INTLINE_OFFSET 0x3c
#define PCI_CONF_INTPIN_OFFSET 0x3d

#define PCI_CONF_INVALID_DEVICE 0xFFFF

/**
 * Base Classes
 */

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

/**
 * Command register bits
 */

#define PCIM_CMD_INTERRUPT_DISABLE		10
#define PCIM_CMD_FAST_BACK_TO_BACK_EN	9
#define PCIM_CMD_SERR_ENABLE			8
#define PCIM_CMD_RESERVED				7
#define PCIM_CMD_PARITY_ERROR_RESPONSE	6
#define PCIM_CMD_VGA_PALETTE_SNOOP		5
#define PCIM_CMD_MEMORY_WR_INVAL_ENABLE	4
#define PCIM_CMD_SPECIAL_CYCLES			3
#define PCIM_CMD_BUS_MASTER				2
#define PCIM_CMD_MEMORY_SPACE			1
#define PCIM_CMD_IO_SPACE				0

/**
 * BAR
 */
#define PCIM_BAR_MEM_IND	0x01
#define PCIM_BAR_MEM(x)		!PCIM_BAR_IO(x)
#define PCIM_BAR_IO(x)		(x & PCIM_BAR_MEM_IND)

// Memory BAR
#define PCIM_BAR_MEM_TYPE_BIT	1
#define PCIM_BAR_MEM_TYPE_32	0
#define PCIM_BAR_MEM_TYPE_64	2
#define PCIM_BAR_PREFETCH_BIT	3
#define PCIM_BAR_MEM_32BASE(x)	(x & 0xFFFFFFF0)
#define PCIM_BAR_MEM_SIZE(x)	(~(uint)(x & 0xFFFFFFF0) + 1)

//IOPort BAR
#define PCIM_BAR_PORT(x)	(x & 0xFFFFFFFC)
#define PCIM_BAR_PORT_SIZE(x)	(~(uint)(x & 0xFFFFFFFE) + 1)

/**
 * Data definition - Classes
 */

CLASSES_DEF(deprecated[], t_dev_info_ptr) {
	{0x00	,DEV_INFO("The old device(non-VGA)")},
	{0x01	,DEV_INFO("The old device(VGA)")},
	{CLASSES_LAST_KEY	,DEV_INFO("")}
};

CLASSES_DEF(storage[], t_dev_info_ptr) {
	{0x00	,DEV_INFO("SCSI bus controller")},
	{0x01	,DEV_INFO("IDE controller")},
	{0x02	,DEV_INFO("Floppy disk controller")},
	{0x03	,DEV_INFO("IPI bus controller")},
	{0x04	,DEV_INFO("RAID controller")},
	{0x80	,DEV_INFO("Other mass storage controller")},
	{CLASSES_LAST_KEY	,DEV_INFO("")}
};

CLASSES_DEF(network[], t_dev_info_ptr) {
	{0x00	,DEV_INFO("Ethernet controller")},
	{0x01	,DEV_INFO("Token Ring controller")},
	{0x02	,DEV_INFO("FDDI controller")},
	{0x03	,DEV_INFO("ATM controller")},
	{0x80	,DEV_INFO("Other network controller")},
	{CLASSES_LAST_KEY	,DEV_INFO("")}
};

CLASSES_DEF(display[], t_dev_info_ptr) {
	{0x00	,DEV_INFO("VGA-compatible controller")},
	{0x01	,DEV_INFO("XGA controller")},
	{0x80	,DEV_INFO("Other display controller")},
	{CLASSES_LAST_KEY	,DEV_INFO("")}
};

CLASSES_DEF(multimedia[], t_dev_info_ptr) {
	{0x00	,DEV_INFO("Video device")},
	{0x01	,DEV_INFO("Audio device")},
	{0x80	,DEV_INFO("Other multimedia controller")},
	{CLASSES_LAST_KEY	,DEV_INFO("")}
};

CLASSES_DEF(memory[], t_dev_info_ptr) {
	{0x00	,DEV_INFO("RAM")},
	{0x01	,DEV_INFO("Flash")},
	{0x80	,DEV_INFO("Other memory controller")},
	{CLASSES_LAST_KEY	,DEV_INFO("")}
};

CLASSES_DEF(brigde[], t_dev_info_ptr) {
	{0x00	,DEV_INFO("Host bridge")},
	{0x01	,DEV_INFO("ISA bridge")},
	{0x02	,DEV_INFO("EISA bridge")},
	{0x03	,DEV_INFO("MCA bridge")},
	{0x04	,DEV_INFO("PCI-to-PCI bridge")},
	{0x05	,DEV_INFO("PCMCIA bridge")},
	{0x06	,DEV_INFO("NuBus bridge")},
	{0x07	,DEV_INFO("CardBus bridge")},
	{0x80	,DEV_INFO("Other bridge device")},
	{CLASSES_LAST_KEY	,DEV_INFO("")}
};

CLASSES_DEF(communication[], t_dev_info_ptr) {
	{0x00	,DEV_INFO("Serial controller")},
	{0x01	,DEV_INFO("Parallel port")},
	{0x80	,DEV_INFO("Other communication controller")},
	{CLASSES_LAST_KEY	,DEV_INFO("")}
};

CLASSES_DEF(peripheral[], t_dev_info_ptr) {
	{0x00	,DEV_INFO("PIC")},
	{0x01	,DEV_INFO("DMA")},
	{0x02	,DEV_INFO("System Timer")},
	{0x03	,DEV_INFO("RTC controller")},
	{0x80	,DEV_INFO("Other system peripheral")},
	{CLASSES_LAST_KEY	,DEV_INFO("")}
};

CLASSES_DEF(input[], t_dev_info_ptr) {
	{0x00	,DEV_INFO("Keyboard controller")},
	{0x01	,DEV_INFO("Digitizer (pen)")},
	{0x02	,DEV_INFO("Mouse controller")},
	{0x80	,DEV_INFO("Other input controller")},
	{CLASSES_LAST_KEY	,DEV_INFO("")}
};

CLASSES_DEF(docking[], t_dev_info_ptr) {
	{0x00	,DEV_INFO("Generic docking station")},
	{0x80	,DEV_INFO("Other type of docking station")},
	{CLASSES_LAST_KEY	,DEV_INFO("")}
};

CLASSES_DEF(processor[], t_dev_info_ptr) {
	{0x00	,DEV_INFO("386")},
	{0x01	,DEV_INFO("486")},
	{0x02	,DEV_INFO("Pentium")},
	{0x10	,DEV_INFO("Alpha")},
	{0x20	,DEV_INFO("PowerPC")},
	{0x40	,DEV_INFO("Co-processor")},
	{CLASSES_LAST_KEY	,DEV_INFO("")}
};

CLASSES_DEF(serialbus[], t_dev_info_ptr) {
	{0x00	,DEV_INFO("FireWire (IEEE 1394)")},
	{0x01	,DEV_INFO("ACCESS.bus")},
	{0x02	,DEV_INFO("SSA")},
	{0x03	,DEV_INFO("Universal Serial Bus (USB)")},
	{0x04	,DEV_INFO("Fibre Channel")},
	{CLASSES_LAST_KEY	,DEV_INFO("")}
};

VARIABLE_DEF(base_classes[], t_base_class_info_ptr, ) {
	{0x00	,CLASSES_VAR_NAME(deprecated)},
	{0x01	,CLASSES_VAR_NAME(storage)},
	{0x02	,CLASSES_VAR_NAME(network)},
	{0x03	,CLASSES_VAR_NAME(display)},
	{0x04	,CLASSES_VAR_NAME(multimedia)},
	{0x05	,CLASSES_VAR_NAME(memory)},
	{0x06	,CLASSES_VAR_NAME(brigde)},
	{0x07	,CLASSES_VAR_NAME(communication)},
	{0x08	,CLASSES_VAR_NAME(peripheral)},
	{0x09	,CLASSES_VAR_NAME(input)},
	{0x0A	,CLASSES_VAR_NAME(docking)},
	{0x0B	,CLASSES_VAR_NAME(processor)},
	{0x0C	,CLASSES_VAR_NAME(serialbus)},
	{CLASSES_LAST_KEY	,0x00}
};

#endif
