/*
 * Copyright (C) 2008 Sergey Gridassov
 */
#include <fos/printk.h>
#include <fos/system.h>
#include <string.h>
#include "smp.h"
#include <fos/drivers/apic/apic.h>



int SMP::mpf_checksum(u8_t *mp, int len) {
	int sum = 0;

	while(len--)
		sum += *mp++;

	return sum & 0xFF;
}

bool SMP::ScanConfig(u32_t base, u32_t length) {
	u32_t *bp = (u32_t *)base;
	struct intel_mp_floating *mpf;

	if (sizeof(*mpf) != 16)
		printk("Error: MPF size\n");

	while (length > 0) {
		mpf = (struct intel_mp_floating *)bp;
		if ((*bp == SMP_MAGIC_IDENT) &&
			(mpf->mpf_length == 1) &&
			!mpf_checksum((unsigned char *)bp, 16) &&
			((mpf->mpf_specification == 1) || (mpf->mpf_specification == 4))) {

			found_config = true;
			mpf_found = mpf;
			return true;
                 }
                 bp += 4;
                 length -= 16;
         }
	return false;
}

void SMP::ConstructISAMPTable(int mpc_default_type) {
	struct mpc_config_processor processor;
	struct mpc_config_bus bus;
	struct mpc_config_ioapic ioapic;
	struct mpc_config_lintsrc lintsrc;
	int linttypes[2] = { mp_ExtINT, mp_NMI };

	mp_lapic_addr = APIC_DEFAULT_PHYS_BASE;
	
	processor.mpc_type = MP_PROCESSOR;
	processor.mpc_apicver = mpc_default_type > 4 ? 0x10 : 0x01;
	processor.mpc_cpuflag = CPU_ENABLED;
	processor.mpc_cpufeature = (system->cpuid->family << 8) | (system->cpuid->model << 4) | system->cpuid->stepping;
	processor.mpc_featureflag = system->cpuid->features_edx;
	processor.mpc_reserved[0] = 0;
	processor.mpc_reserved[1] = 0; 

	for (int i = 0; i < 2; i++) {
		processor.mpc_apicid = i;
		ProcessorInfo(&processor);
	}

	bus.mpc_type = MP_BUS;
	bus.mpc_busid = 0;
	switch(mpc_default_type) {
	default:
		printk("Unknown standard configuration %d\n", mpc_default_type);
	case 1:
	case 5:
		memcpy(bus.mpc_bustype, "ISA   ", 6);
		break;
	case 2:
	case 6:
	case 3:
		memcpy(bus.mpc_bustype, "EISA  ", 6);
		break;
	case 4:
	case 7:
		memcpy(bus.mpc_bustype, "MCA   ", 6);
	}
	BusInfo(&bus);

	if(mpc_default_type > 4) {
		bus.mpc_busid = 1;
		memcpy(bus.mpc_bustype, "PCI   ", 6);
		BusInfo(&bus);
	}

	ioapic.mpc_type = MP_IOAPIC;
	ioapic.mpc_apicid = 2;
	ioapic.mpc_apicver = mpc_default_type > 4 ? 0x10 : 0x01;
	ioapic.mpc_flags = MPC_APIC_USABLE;
	ioapic.mpc_apicaddr = 0xFEC00000;
	IOAPICInfo(&ioapic);

	ConstructIOIRQMPTable(mpc_default_type);
	lintsrc.mpc_type = MP_LINTSRC;
	lintsrc.mpc_irqflag = 0;		/* conforming */
	lintsrc.mpc_srcbusid = 0;
	lintsrc.mpc_srcbusirq = 0;
	lintsrc.mpc_destapic = MP_APIC_ALL;
	for (int i = 0; i < 2; i++) {
		lintsrc.mpc_irqtype = linttypes[i];
		lintsrc.mpc_destapiclint = i;
		LINTSrcInfo(&lintsrc);
	}
}

void SMP::GetSMPConfig() {
	struct intel_mp_floating *mpf = mpf_found;

	printk("Intel MP Spec. ver. 1.%d\n", mpf->mpf_specification);

	if(mpf->mpf_feature2 & (1 << 7)) {
//		printk(" IMCR and PIC compatibility mode\n");
		pic_mode = true;
	} else {
//		printk(" Virtual Wire compatibility mode\n");
		pic_mode = false;
	}

	if(mpf->mpf_feature1 != 0) {
		printk("Default MP configuration #%d\n",  mpf->mpf_feature1);
		ConstructISAMPTable(mpf->mpf_feature1);
	} else if (mpf->mpf_physptr) {
		mp_config_table *mpc;
		if(mpf->mpf_physptr > 1024 * 1024)
			mpc = (mp_config_table *) system->kmem->mmap(0, 65535, 0, mpf->mpf_physptr, 0);
		else
			mpc = (mp_config_table *)mpf->mpf_physptr;

		if (!ReadMPC(mpc)) 
			system->panic("Broken BIOS (MPC table broken), use isa_legacy\n");

		if(mpf->mpf_physptr > 1024 * 1024)
			system->kmem->munmap((off_t)mpc, 65535);

		if (!mp_irq_entries && 0) {
			struct mpc_config_bus bus;

			printk("BIOS bug, no explicit IRQ entries, using default mptable.\n");

			bus.mpc_type = MP_BUS;
			bus.mpc_busid = 0;
			memcpy(bus.mpc_bustype, "ISA   ", 6);
			BusInfo(&bus);

			ConstructIOIRQMPTable(0);
		}		
	} else 
		system->panic("Broken BIOS (no MPC table), use isa_legacy\n");

}

SMP::SMP() {
	found_config = false;
	pic_mode = false;
	mpf_found = NULL;
	mp_lapic_addr = 0;
	mp_irq_entries = 0;
	boot_cpu_physical_apicid = -1;
	num_processors = 0;

	for(int i = 0; i < NR_CPUS; i++) 
		bios_cpu_apicid[i] = BAD_APICID;

	for(int i = 0; i < MAX_MP_BUSSES; i++)
		mp_bus_id_to_pci_bus[i] = -1;

	mp_current_pci_id = 0;
	nr_ioapics = 0;
	mpc_record = 0;

	if(!(ScanConfig(0x0, 0x400) || ScanConfig(639 * 0x400, 0x400) || ScanConfig(0xF0000, 0x10000)))
	{
		u16_t address = *(u16_t *)0x40E;
		address <<=4;
		if(address) 
			ScanConfig(address, 0x400);
	}
	
	if(found_config) {
//		printk("SMP system detected\n");
		GetSMPConfig();
	} else
		printk("Uniprocessor system detected\n");
}

u8_t SMP::ELCR_trigger(int irq) {
	u16_t port = 0x4d0 + (irq >> 3);
	return (system->inportb(port) >> (irq & 7)) & 1;
}

void SMP::INTSrcInfo(struct mpc_config_intsrc *m) {
	mp_irqs[mp_irq_entries] = *m;

//	printk(	"Int: type %d, pol %d, trig %d, bus %d, "
//		"IRQ 0x%02x, APIC ID 0x%02x, APIC INT 0x%02x\n",
//			m->mpc_irqtype, m->mpc_irqflag & 3,
//			(m->mpc_irqflag >> 2) & 3, m->mpc_srcbus,
//			m->mpc_srcbusirq, m->mpc_dstapic, m->mpc_dstirq);
	if (++mp_irq_entries == MAX_IRQ_SOURCES)
		system->panic("SMP: Max # of irq sources exceeded");
}

void SMP::ProcessorInfo(struct mpc_config_processor *m) {
	if (!(m->mpc_cpuflag & CPU_ENABLED))
		return;

//	printk("CPU %d", m->mpc_apicid);
	if (m->mpc_cpuflag & CPU_BOOTPROCESSOR) {
//		printk(" Bootup CPU");
		boot_cpu_physical_apicid = m->mpc_apicid;
	}
//	printk("\n");
	int ver = m->mpc_apicver;

	if(ver == 0) {
		printk("BIOS bug, APIC version is 0 for CPU#%d, fixing to 0x10.\n", m->mpc_apicid);
		ver = 0x10;
	}

	if (num_processors >= NR_CPUS) {
		printk("WARNING: NR_CPUS limit of %i reached. Processor ignored.\n", NR_CPUS);
		return;
	}

	bios_cpu_apicid[num_processors] = m->mpc_apicid;

	num_processors++;

}

void SMP::ConstructIOIRQMPTable(int mpc_default_type) {
	struct mpc_config_intsrc intsrc;
	bool ELCR_fallback = false;

	intsrc.mpc_type = MP_INTSRC;
	intsrc.mpc_irqflag = 0;
	intsrc.mpc_srcbus = 0;
	intsrc.mpc_dstapic = mp_ioapics[0].mpc_apicid;

	intsrc.mpc_irqtype = mp_INT;

	if(mpc_default_type == 5) {
		printk("ISA/PCI bus type with no IRQ information, falling back to ELCR\n");

		if (ELCR_trigger(0) || ELCR_trigger(1) || ELCR_trigger(2) || ELCR_trigger(13))
			printk("ELCR contains invalid data,  not using ELCR\n");
		else {
			printk("Using ELCR to identify PCI interrupts\n");
			ELCR_fallback = true;
		}
	}
	for(int i = 0; i < 16; i++) {
		switch(mpc_default_type) {
		case 2:
			if(i == 0 || i == 13)
				continue;
		default:
			if(i == 2)
				continue;
		}
		
		if(ELCR_fallback) {
			if (ELCR_trigger(i))
				intsrc.mpc_irqflag = 13;
			else
				intsrc.mpc_irqflag = 0;
		}
		intsrc.mpc_srcbusirq = i;
		intsrc.mpc_dstirq = i ? i : 2;		/* IRQ0 to INTIN2 */
		INTSrcInfo(&intsrc);
	}
	intsrc.mpc_irqtype = mp_ExtINT;
	intsrc.mpc_srcbusirq = 0;
	intsrc.mpc_dstirq = 0;				/* 8259A to INTIN0 */
	INTSrcInfo(&intsrc);
}

void SMP::BusInfo(struct mpc_config_bus *m) {
	char str[7];

	memcpy(str, m->mpc_bustype, 6);
	str[6] = 0;

//	printk("Bus #%d: %s\n", m->mpc_busid, str);

	if (strncmp(str, BUSTYPE_ISA, sizeof(BUSTYPE_ISA)-1) == 0) {
		mp_bus_id_to_type[m->mpc_busid] = MP_BUS_ISA;
	} else if (strncmp(str, BUSTYPE_EISA, sizeof(BUSTYPE_EISA)-1) == 0) {
		mp_bus_id_to_type[m->mpc_busid] = MP_BUS_EISA;
	} else if (strncmp(str, BUSTYPE_PCI, sizeof(BUSTYPE_PCI)-1) == 0) {
		mp_bus_id_to_type[m->mpc_busid] = MP_BUS_PCI;
		mp_bus_id_to_pci_bus[m->mpc_busid] = mp_current_pci_id;
		mp_current_pci_id++;
	} else if (strncmp(str, BUSTYPE_MCA, sizeof(BUSTYPE_MCA)-1) == 0) {
		mp_bus_id_to_type[m->mpc_busid] = MP_BUS_MCA;
	} else {
		printk("Unknown bustype %s - ignoring\n", str);
	}

}

void SMP::IOAPICInfo(struct mpc_config_ioapic *m) {
	if (!(m->mpc_flags & MPC_APIC_USABLE))
		return;

//	

	if (nr_ioapics >= MAX_IO_APICS) {
		system->panic("Max # of I/O APICs (%d) exceeded (found %d).\n",	MAX_IO_APICS, nr_ioapics);
	}
	if (!m->mpc_apicaddr) {
		printk("WARNING: bogus zero I/O APIC address found in MP table, skipping!\n");
		return;
	}
	mp_ioapics[nr_ioapics] = *m;

	ioapics_regs[nr_ioapics] = system->kmem->mmap(0, 4096, 0, mp_ioapics[nr_ioapics].mpc_apicaddr, 0);

	printk("I/O APIC #%d Version %d at 0x%08X, mapped to 0x%08X\n", m->mpc_apicid, m->mpc_apicver, m->mpc_apicaddr, ioapics_regs[nr_ioapics]);

	nr_ioapics++;
}

void SMP::LINTSrcInfo(struct mpc_config_lintsrc *m)
{
//	printk("Lint: type %d, pol %d, trig %d, bus %d,"
//		" IRQ %02x, APIC ID %x, APIC LINT %02x\n",
//			m->mpc_irqtype, m->mpc_irqflag & 3,
//			(m->mpc_irqflag >> 2) &3, m->mpc_srcbusid,
//			m->mpc_srcbusirq, m->mpc_destapic, m->mpc_destapiclint);
}

bool SMP::ReadMPC(mp_config_table *mpc) {
	char str[16], oem[10];
	int count = sizeof(*mpc);

	u8_t *mpt = (u8_t *) mpc + count;

	if(memcmp(mpc->mpc_signature, MPC_SIGNATURE, 4)) {
		printk("SMP mptable: bad signature 0x%08X\n", *(u32_t *)mpc->mpc_signature);
		return false;
	}

	if (mpf_checksum((u8_t *) mpc, mpc->mpc_length)) {
		printk("SMP mptable: checksum error\n");
		return false;
	}
	if (mpc->mpc_spec != 0x01 && mpc->mpc_spec != 0x04) {
		printk("SMP mptable: bad table version %d\n", mpc->mpc_spec);
		return false;
	}
	if (mpc->mpc_lapic == 0) {
		printk("SMP mptable: null local APIC address\n");
		return false;
	}

	memcpy(oem,mpc->mpc_oem,8);
	oem[8]=0;
	printk(" OEM ID: %s ",oem);

	memcpy(str,mpc->mpc_productid,12);
	str[12]=0;
	printk("Product ID: %s ",str);

	printk("APIC at: 0x%lX\n",mpc->mpc_lapic);

	mp_lapic_addr = mpc->mpc_lapic;

	mpc_record = 0;
	while (count < mpc->mpc_length) {
		switch(*mpt) {
			case MP_PROCESSOR:
			{
				struct mpc_config_processor *m = (struct mpc_config_processor *)mpt;
				ProcessorInfo(m);
				mpt += sizeof(*m);
				count += sizeof(*m);
				break;
			}
			case MP_BUS:
			{
				struct mpc_config_bus *m = (struct mpc_config_bus *)mpt;
				BusInfo(m);
				mpt += sizeof(*m);
				count += sizeof(*m);
				break;
			}
			case MP_IOAPIC:
			{
				struct mpc_config_ioapic *m = (struct mpc_config_ioapic *)mpt;
				IOAPICInfo(m);
				mpt += sizeof(*m);
				count += sizeof(*m);
				break;
			}
			case MP_INTSRC:
			{
				struct mpc_config_intsrc *m = (struct mpc_config_intsrc *)mpt;

				INTSrcInfo(m);
				mpt+=sizeof(*m);
				count+=sizeof(*m);
				break;
			}
			case MP_LINTSRC:
			{
				struct mpc_config_lintsrc *m=  (struct mpc_config_lintsrc *)mpt;
				LINTSrcInfo(m);
				mpt+=sizeof(*m);
				count+=sizeof(*m);
				break;
			}
			default:
			{
				count = mpc->mpc_length;
				break;
			}
		}
		++mpc_record;
	}

	if (!num_processors) {
		printk( "SMP mptable: no processors registered!\n");
		return false;
	}
	return true;
}

int SMP::FindISAIRQPin(int irq, int type) {
	for (int i = 0; i < mp_irq_entries; i++) {
		int lbus = mp_irqs[i].mpc_srcbus;

		if ((mp_bus_id_to_type[lbus] == MP_BUS_ISA ||
		     mp_bus_id_to_type[lbus] == MP_BUS_EISA ||
		     mp_bus_id_to_type[lbus] == MP_BUS_MCA
		    ) &&
		    (mp_irqs[i].mpc_irqtype == type) &&
		    (mp_irqs[i].mpc_srcbusirq == irq))

			return mp_irqs[i].mpc_dstirq;
	}
	return -1;
}

int SMP::FindISAIRQAPIC(int irq, int type) {
	int i = 0;
	for (i = 0; i < mp_irq_entries; i++) {
		int lbus = mp_irqs[i].mpc_srcbus;

		if ((mp_bus_id_to_type[lbus] == MP_BUS_ISA ||
		     mp_bus_id_to_type[lbus] == MP_BUS_EISA ||
		     mp_bus_id_to_type[lbus] == MP_BUS_MCA
		    ) &&
		    (mp_irqs[i].mpc_irqtype == type) &&
		    (mp_irqs[i].mpc_srcbusirq == irq))
			break;
	}
	if (i < mp_irq_entries) {
		int apic;
		for(apic = 0; apic < nr_ioapics; apic++) {
			if (mp_ioapics[apic].mpc_apicid == mp_irqs[i].mpc_dstapic)
				return apic;
		}
	}

	return -1;
}
