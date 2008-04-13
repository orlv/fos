/*
  kernel/drivers/apic/ioapic.cpp
  Copyright (C) 2008 Sergey Gridassov
*/

#include "ioapic.h"
#include <fos/system.h>
#include <fos/printk.h>
#include <string.h>

inline u32_t *IOAPIC::ioapic_base(int idx) {
	return (u32_t *) system->smp->ioapics_regs[idx];
}

inline u32_t IOAPIC::ioapic_read(u32_t apic, u32_t reg) {
	u32_t * volatile  ptr = ioapic_base(apic);
	ptr[0] = reg;
	return ptr[4];
}


inline void IOAPIC::ioapic_write(u32_t apic, u32_t reg, u32_t val) {
	u32_t * volatile  ptr = ioapic_base(apic);
	ptr[0] = reg;
	ptr[4] = val;
}


void IOAPIC::EnableIOAPIC () {
	union IO_APIC_reg_01 reg_01;

	for(int i = 0; i < PIN_MAP_SIZE; i++) {
		irq_2_pin[i].pin = -1;
		irq_2_pin[i].next = 0;
	}

	for(int i = 0; i < MAX_PIRQS; i++)
		pirq_entries[i] = -1;

	for(int apic = 0; apic < system->smp->nr_ioapics; apic++) {
		reg_01.raw = ioapic_read(apic, 1);
		nr_ioapic_registers[apic] = reg_01.bits.entries+1;
	}
	for(int apic = 0; apic < system->smp->nr_ioapics; apic++) {
		for (int pin = 0; pin < nr_ioapic_registers[apic]; pin++) {
			struct IO_APIC_route_entry entry;
			entry = ReadEntry(apic, pin);

			if ((entry.mask == 0) && (entry.delivery_mode == dest_ExtINT)) {
				ioapic_i8259.apic = apic;
				ioapic_i8259.pin  = pin;
				goto found_i8259;
			}
		}
	}
found_i8259:

	int i8259_pin  = system->smp->FindISAIRQPin(0, mp_ExtINT);
	int i8259_apic = system->smp->FindISAIRQAPIC(0, mp_ExtINT);
	/* Trust the MP table if nothing is setup in the hardware */
	if ((ioapic_i8259.pin == -1) && (i8259_pin >= 0)) {
		printk("IOAPIC: ExtINT not setup in hardware but reported by MP table\n");
		ioapic_i8259.pin  = i8259_pin;
		ioapic_i8259.apic = i8259_apic;
	}

	if (((ioapic_i8259.apic != i8259_apic) || (ioapic_i8259.pin != i8259_pin)) &&
		(i8259_pin >= 0) && (ioapic_i8259.pin >= 0))
	{
		printk("IOAPIC: ExtINT in hardware and MP table differ\n");
	}

	ClearIOAPIC();
}

void IOAPIC::ClearIOAPIC() {
	for(int apic = 0; apic < system->smp->nr_ioapics; apic++)
		for(int pin = 0; pin < nr_ioapic_registers[apic]; pin++)
			ClearPin(apic, pin);
}

void IOAPIC::MaskEntry(int apic, int pin) {
	union entry_union eu;
	eu.entry.mask = 1;
	ioapic_write(apic, 0x10 + 2 * pin, eu.w1);
	ioapic_write(apic, 0x11 + 2 * pin, eu.w2);
}

void IOAPIC::ClearPin(int apic, int pin) {
	struct IO_APIC_route_entry entry;

	entry = ReadEntry(apic, pin);
	if(entry.delivery_mode == dest_SMI)
		return;

	MaskEntry(apic, pin);
}

IOAPIC::IOAPIC(APIC *apic) {
	parent = apic;
	ioapic_i8259.pin = -1;
	ioapic_i8259.apic = -1;
	current_vector = 0x21;
	current_offset = 0;
	irq_vector[0] = 0x21;
	irq_vector[1] = 0;
	EnableIOAPIC();

	io_apic_irqs = ~(1 << PIC_CASCADE_IR);

	printk("IOAPIC: Enabling IO-APIC IRQs\n");

	SetupIDsFromMPC();
	parent->SyncArbIDs();
	SetupIRQs();

}

struct IO_APIC_route_entry IOAPIC::ReadEntry(int apic, int pin) {
	union entry_union eu;
	eu.w1 = ioapic_read(apic, 0x10 + 2 * pin);
	eu.w2 = ioapic_read(apic, 0x11 + 2 * pin);
	return eu.entry;
}

void IOAPIC::SetupIDsFromMPC() {
	union IO_APIC_reg_00 reg_00;

	if(system->cpuid->vendor_code != VENDOR_INTEL || APIC_XAPIC(system->smp->apic_version[system->smp->boot_cpu_physical_apicid])) {
		printk("xAPIC detected, skipping setting IO-APIC IDs\n");
		return;
	}

	for (int apic = 0; apic < system->smp->nr_ioapics; apic++) {
		reg_00.raw = ioapic_read(apic, 0);

		int old_id = system->smp->mp_ioapics[apic].mpc_apicid;

		if (old_id >= parent->GetPhysicalBroadcast()) {
			printk("BIOS bug, IO-APIC#%d ID is %d in the MPC table, fixing up to %d\n",
				apic, old_id, reg_00.bits.ID);
			system->smp->mp_ioapics[apic].mpc_apicid = reg_00.bits.ID;
		}

		if (old_id != system->smp->mp_ioapics[apic].mpc_apicid)
			for (int i = 0; i < system->smp->mp_irq_entries; i++)
				if (system->smp->mp_irqs[i].mpc_dstapic == old_id)
					system->smp->mp_irqs[i].mpc_dstapic = system->smp->mp_ioapics[apic].mpc_apicid;

		printk(	"Changing IO-APIC physical APIC ID to %d ...", system->smp->mp_ioapics[apic].mpc_apicid);

		reg_00.bits.ID = system->smp->mp_ioapics[apic].mpc_apicid;
		ioapic_write(apic, 0, reg_00.raw);

		reg_00.raw = ioapic_read(apic, 0);

		if (reg_00.bits.ID != system->smp->mp_ioapics[apic].mpc_apicid)
			printk("could not set ID!\n");
		else
			printk(" ok.\n");
	}
}

void IOAPIC::disable_8259A_irq(int n)
{
  if (n > 15)
    return;

  u8_t port;
  
  if (n > 7) {
    n -= 8;
    port = 0xA1;
  } else
    port = 0x21;

  system->outportb(port, system->inportb(port) | (1<<n));
}

void IOAPIC::SetupIRQs() {
	struct IO_APIC_route_entry entry;
	bool first_notcon = true;
	printk("init IOAPIC IRQs\n");
	for(int apic = 0; apic < system->smp->nr_ioapics; apic++)
	for(int pin = 0; pin < nr_ioapic_registers[apic]; pin++) {
		memset(&entry, 0, sizeof(entry));
		entry.delivery_mode = dest_LowestPrio;
		entry.dest_mode = 1;
		entry.mask = 0;				/* enable IRQ */
		entry.dest.logical.logical_dest = 0;

		int idx = system->smp->FindIRQEntry(apic, pin, mp_INT);
		if(idx == -1) {
			if(first_notcon) {
				printk(" IO-APIC (apicid-pin) %d-%d", system->smp->mp_ioapics[apic].mpc_apicid, pin);
				first_notcon = false;
			} else
				printk(", %d-%d", system->smp->mp_ioapics[apic].mpc_apicid, pin);
			continue;
		}
		
		entry.trigger = system->smp->MPBIOS_trigger(idx);
		entry.polarity = system->smp->MPBIOS_polarity(idx);

		if (entry.trigger) {
			entry.trigger = 1;
			entry.mask = 1;
		}

		int irq = Pin2IRQ(idx, apic, pin);

		AddPinToIRQ(irq, apic, pin);

		if (apic == 0 && !IO_APIC_IRQ(irq))
			continue;

		if (IO_APIC_IRQ(irq)) {
			int vector = AssignIRQVector(irq);
			entry.vector = vector;
		
			if (apic == 0 && irq < 16)
				disable_8259A_irq(irq);
		}
		WriteEntry(apic, pin, entry);
	}

}

int IOAPIC::Pin2IRQ(int idx, int apic, int pin) {
	int bus = system->smp->mp_irqs[idx].mpc_srcbus;

	if(system->smp->mp_irqs[idx].mpc_dstirq != pin)
		system->panic("Broken BIOS or MPTABLE parser, use 'isa_legacy'\n");

	int irq;

	switch (system->smp->mp_bus_id_to_type[bus])
	{
		case MP_BUS_ISA: 
		case MP_BUS_EISA:
		case MP_BUS_MCA:
		{
			irq = system->smp->mp_irqs[idx].mpc_srcbusirq;
			break;
		}
		case MP_BUS_PCI:
		{

			for(int i = irq = 0; i < apic; )
				irq += nr_ioapic_registers[i++];
			irq += pin;


			break;
		}
		default:
		{
			printk("SMP: unknown bus type %d.\n",bus); 
			irq = 0;
			break;
		}
	}

	if ((pin >= 16) && (pin <= 23)) {
		if (pirq_entries[pin-16] != -1) {
			if (!pirq_entries[pin-16]) {
				printk("APIC: disabling PIRQ%d\n", pin-16);
			} else {
				irq = pirq_entries[pin-16];
				printk("APIC: using PIRQ%d -> IRQ %d\n", pin - 16, irq);
			}
		}
	}
	return irq;	
}

void IOAPIC::AddPinToIRQ(int irq, int apic, int pin)
{
	static int first_free_entry = NR_IRQS;
	struct irq_pin_list *entry = irq_2_pin + irq;

	while (entry->next)
		entry = irq_2_pin + entry->next;

	if (entry->pin != -1) {
		entry->next = first_free_entry;
		entry = irq_2_pin + entry->next;
		if (++first_free_entry >= PIN_MAP_SIZE)
			system->panic("IOAPIC::AddPinToIRQ: oops");
	}
	entry->apic = apic;
	entry->pin = pin;
}

int IOAPIC::AssignIRQVector(int irq) {

	printk("[irq %d]", irq);

	int vector, offset, i;

	if(irq >= 224)
		system->panic("Too big IRQ %d\n", irq);

	if (irq_vector[irq] > 0)
		return irq_vector[irq];

	vector = current_vector;
	offset = current_offset;
next:
	printk(".");
	vector += 8;
	if (vector >= 0x21) {
		offset = (offset + 1) % 8;
		vector = 0x21 + offset;
	}
	if (vector == current_vector)
		system->panic("No space for IRQ\n");
	
	for (i = 0; i < 224; i++)
		if (irq_vector[i] == vector)
			goto next;


	printk("[vector %x]\n", vector);

	current_vector = vector;
	current_offset = offset;
	irq_vector[irq] = vector;

	return vector;
}

void IOAPIC::WriteEntry(int apic, int pin, struct IO_APIC_route_entry e) {
	union entry_union eu;
	eu.entry = e;
	ioapic_write(apic, 0x11 + 2*pin, eu.w2);
	ioapic_write(apic, 0x10 + 2*pin, eu.w1);
}
