/*
  kernel/drivers/apic/ioapic_defs.h
  Copyright (C) 2008 Sergey Gridassov
*/

#ifndef __IOAPIC_DEFS_H
#define __IOAPIC_DEFS_H

#define NR_IRQS 224
#define MAX_PIRQS 8

#define MAX_PLUS_SHARED_IRQS NR_IRQS
#define PIN_MAP_SIZE (MAX_PLUS_SHARED_IRQS + NR_IRQS)

struct io_apic {
	volatile unsigned int index;
	unsigned int unused[3];
	volatile unsigned int data;
};

union IO_APIC_reg_00 {
	u32_t	raw;
	struct {
		u32_t	__reserved_2	: 14,
			LTS		:  1,
			delivery_type	:  1,
			__reserved_1	:  8,
			ID		:  8;
	} __attribute__ ((packed)) bits;
};

union IO_APIC_reg_01 {
	u32_t	raw;
	struct {
		u32_t	version		:  8,
			__reserved_2	:  7,
			PRQ		:  1,
			entries		:  8,
			__reserved_1	:  8;
	} __attribute__ ((packed)) bits;
};

union IO_APIC_reg_02 {
	u32_t	raw;
	struct {
		u32_t	__reserved_2	: 24,
			arbitration	:  4,
			__reserved_1	:  4;
	} __attribute__ ((packed)) bits;
};

union IO_APIC_reg_03 {
	u32_t	raw;
	struct {
		u32_t	boot_DT		:  1,
			__reserved_1	: 31;
	} __attribute__ ((packed)) bits;
};
enum ioapic_irq_destination_types {
	dest_Fixed = 0,
	dest_LowestPrio = 1,
	dest_SMI = 2,
	dest__reserved_1 = 3,
	dest_NMI = 4,
	dest_INIT = 5,
	dest__reserved_2 = 6,
	dest_ExtINT = 7
};

struct IO_APIC_route_entry {
	u32_t	vector		:  8,
		delivery_mode	:  3,	/* 000: FIXED
					 * 001: lowest prio
					 * 111: ExtINT
					 */
		dest_mode	:  1,	/* 0: physical, 1: logical */
		delivery_status	:  1,
		polarity	:  1,
		irr		:  1,
		trigger		:  1,	/* 0: edge, 1: level */
		mask		:  1,	/* 0: enabled, 1: disabled */
		__reserved_2	: 15;

	union {		struct { u32_t
					__reserved_1	: 24,
					physical_dest	:  4,
					__reserved_2	:  4;
			} physical;

			struct { u32_t
					__reserved_1	: 24,
					logical_dest	:  8;
			} logical;
	} dest;

} __attribute__ ((packed));

union entry_union {
	struct { u32_t w1, w2; };
	struct IO_APIC_route_entry entry;
};

#endif
