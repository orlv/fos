#include "scan.h"
const pci_dev dev_class[] = {
	{ BASE_CLASS_DEPRECATED, 0x00, "Deprecated non-VGA device" },
	{ BASE_CLASS_DEPRECATED, 0x01, "Deprecated VGA device" },

	{ BASE_CLASS_STORAGE, 0x00, "SCSI bus controller" },
	{ BASE_CLASS_STORAGE, 0x01, "IDE controller" },
	{ BASE_CLASS_STORAGE, 0x02, "Floppy disk controller" },
	{ BASE_CLASS_STORAGE, 0x03, "IPI bus controller" },
	{ BASE_CLASS_STORAGE, 0x04, "RAID controller" },
	{ BASE_CLASS_STORAGE, 0x80, "Other mass storage controller" },

	{ BASE_CLASS_NETWORK, 0x00, "Ethernet controller" },
	{ BASE_CLASS_NETWORK, 0x01, "Token Ring controller" },
	{ BASE_CLASS_NETWORK, 0x02, "FDDI controller" },
	{ BASE_CLASS_NETWORK, 0x03, "ATM controller" },
	{ BASE_CLASS_NETWORK, 0x80, "Other network controller" },

	{ BASE_CLASS_DISPLAY, 0x00, "VGA-compatible controller" },
	{ BASE_CLASS_DISPLAY, 0x01, "XGA controller" },
	{ BASE_CLASS_DISPLAY, 0x80, "Other display controller"},

	{ BASE_CLASS_MULTIMEDIA, 0x00, "Video device" },
	{ BASE_CLASS_MULTIMEDIA, 0x80, "Audio device" },

	{ BASE_CLASS_MEMORY, 0x00, "RAM" },
	{ BASE_CLASS_MEMORY, 0x01, "Flash" },
	{ BASE_CLASS_MEMORY, 0x80, "Other memory controller" },

	{ BASE_CLASS_BRIDGE, 0x00, "Host bridge" },
	{ BASE_CLASS_BRIDGE, 0x01, "ISA bridge" },
	{ BASE_CLASS_BRIDGE, 0x02, "EISA bridge" },
	{ BASE_CLASS_BRIDGE, 0x03, "MCA bridge" },
	{ BASE_CLASS_BRIDGE, 0x04, "PCI-to-PCI bridge" },
	{ BASE_CLASS_BRIDGE, 0x05, "PCMCIA bridge" },
	{ BASE_CLASS_BRIDGE, 0x06, "NuBus bridge" },
	{ BASE_CLASS_BRIDGE, 0x07, "CardBus bridge" },
	{ BASE_CLASS_BRIDGE, 0x80, "Other bridge device" },

	{ BASE_CLASS_COMMUN, 0x00, "Serial controller"},
	{ BASE_CLASS_COMMUN, 0x01, "Parallel port"},
	{ BASE_CLASS_COMMUN, 0x80, "Other communication controller"},

	{ BASE_CLASS_BASE_SYS_BUS, 0x00, "PIC"},
	{ BASE_CLASS_BASE_SYS_BUS, 0x01, "DMA"},
	{ BASE_CLASS_BASE_SYS_BUS, 0x02, "System Timer"},
	{ BASE_CLASS_BASE_SYS_BUS, 0x03, "RTC controller"},
	{ BASE_CLASS_BASE_SYS_BUS, 0x80, "Other system peripheral"},

	{ BASE_CLASS_DOCKING, 0x00, "Generic docking station" },
	{ BASE_CLASS_DOCKING, 0x80, "Other type of docking station" },

	{ BASE_CLASS_PROCESSOR, 0x00, "386" },
	{ BASE_CLASS_PROCESSOR, 0x01, "486" },
	{ BASE_CLASS_PROCESSOR, 0x02, "Pentium" },
	{ BASE_CLASS_PROCESSOR, 0x10, "Alpha" },
	{ BASE_CLASS_PROCESSOR, 0x20, "PowerPC" },
	{ BASE_CLASS_PROCESSOR, 0x40, "Co-processor" },
	
	{ BASE_CLASS_SERIAL_BUS, 0x00, "FireWire (IEEE 1394)" },
	{ BASE_CLASS_SERIAL_BUS, 0x01, "ACCESS.bus" },
	{ BASE_CLASS_SERIAL_BUS, 0x02, "SSA" },
	{ BASE_CLASS_SERIAL_BUS, 0x03, "Universal Serial Bus (USB)" },
	{ BASE_CLASS_SERIAL_BUS, 0x04, "Fibre Channel" },
};

char *find_device(int class) {
	int base_class = (class >> 16) ;
	int sub_class = (class >> 8) & 0x0FF;
//	int interface_class = class & 0xFF;
	for(int i = 0; i < sizeof(dev_class) / sizeof(pci_dev); i++) {
		if(dev_class[i].base_class == base_class && 
		dev_class[i].sub_class == sub_class)
			return dev_class[i].text;
	}
	return "Unknown device";
}
