/*
  kernel/drivers/apic/ioapic.h
  Copyright (C) 2008 Sergey Gridassov
*/

#ifndef __IOAPIC_H
#define __IOAPIC_H

#include <fos/system.h>
#include <fos/drivers/smp/smp.h>

#include "ioapic_defs.h"

class IOAPIC {
private:
	void EnableIOAPIC();
	struct IO_APIC_route_entry ReadEntry(int apic, int pin);

	static inline u32_t ioapic_read(u32_t apic, u32_t reg);
	static inline u32_t *ioapic_base(int idx);

	struct irq_pin_list {
		int apic, pin, next;
	} irq_2_pin[PIN_MAP_SIZE];
	int nr_ioapic_registers[MAX_IO_APICS];
	int pirq_entries [MAX_PIRQS];

	struct { int pin, apic; } ioapic_i8259;

public:
	IOAPIC();
};

#endif
