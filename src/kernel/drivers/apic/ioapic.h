/*
  kernel/drivers/apic/ioapic.h
  Copyright (C) 2008 Sergey Gridassov
*/

#ifndef __IOAPIC_H
#define __IOAPIC_H

#include <fos/system.h>
#include <fos/drivers/smp/smp.h>
#include <fos/drivers/smp/smp_defs.h>

class IOAPIC;
class APIC;

#include "apic.h"
#include "ioapic_defs.h"

class IOAPIC {
private:
	APIC *parent;
	void EnableIOAPIC();
	struct IO_APIC_route_entry ReadEntry(int apic, int pin);
	void ClearIOAPIC();
	void ClearPin(int apic, int pin);
	void MaskEntry(int apic, int pin);
	void SetupIDsFromMPC();
	void SetupIRQs();
	int Pin2IRQ(int idx, int apic, int pin);
	void AddPinToIRQ(int irq, int apic, int pin);
	int AssignIRQVector(int irq);

	inline u32_t ioapic_read(u32_t apic, u32_t reg);
	inline u32_t *ioapic_base(int idx);
	inline void ioapic_write(u32_t apic, u32_t reg, u32_t val);

	struct irq_pin_list {
		int apic, pin, next;
	} irq_2_pin[PIN_MAP_SIZE];
	int nr_ioapic_registers[MAX_IO_APICS];
	int pirq_entries [MAX_PIRQS];
	int io_apic_irqs;
	u8_t irq_vector[224];
	struct { int pin, apic; } ioapic_i8259;
	void disable_8259A_irq(int n);
	void WriteEntry(int apic, int pin, struct IO_APIC_route_entry e);

public:
	IOAPIC(APIC *apic);
};

#endif
