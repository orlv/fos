/*
  kernel/drivers/apic/ioapic.cpp
  Copyright (C) 2008 Sergey Gridassov
*/

#include "ioapic.h"
#include <fos/system.h>
#include <fos/printk.h>

inline u32_t *IOAPIC::ioapic_base(int idx) {
	return (u32_t *) system->smp->ioapics_regs[idx];
}

inline u32_t IOAPIC::ioapic_read(u32_t apic, u32_t reg) {
	u32_t * volatile  ptr = ioapic_base(apic);
	ptr[0] = reg;
	return ptr[4];
}

void IOAPIC::EnableIOAPIC () {
	union IO_APIC_reg_01 reg_01;
	printk("IOAPIC: Enabling\n");

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
		printk("ExtINT in hardware and MP table differ\n");
	}

}

IOAPIC::IOAPIC() {
	ioapic_i8259.pin = -1;
	ioapic_i8259.apic = -1;
	EnableIOAPIC();
}

struct IO_APIC_route_entry IOAPIC::ReadEntry(int apic, int pin) {
	union entry_union eu;
	eu.w1 = ioapic_read(apic, 0x10 + 2 * pin);
	eu.w2 = ioapic_read(apic, 0x11 + 2 * pin);
	return eu.entry;
}
