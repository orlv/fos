/*
 * Copyright (C) 2008 Sergey Gridassov
 */

#ifndef SMP_H
#define SMP_H

#include "smp_defs.h"


class SMP {
	bool ScanConfig(u32_t base, u32_t len);
	int mpf_checksum(u8_t *mp, int len);
	struct intel_mp_floating *mpf_found;
	void GetSMPConfig();
	void ConstructISAMPTable(int mpc_default_type);
	void ProcessorInfo(struct mpc_config_processor *m);
	void BusInfo(struct mpc_config_bus *m);
	void IOAPICInfo(struct mpc_config_ioapic *m);
	void LINTSrcInfo(struct mpc_config_lintsrc *m);
	void ConstructIOIRQMPTable(int mpc_default_type);
	u8_t ELCR_trigger(int irq);
	void INTSrcInfo(struct mpc_config_intsrc *m);
	bool ReadMPC(mp_config_table *mpc);

	struct mpc_config_ioapic mp_ioapics[MAX_IO_APICS];
	struct mpc_config_intsrc mp_irqs[MAX_IRQ_SOURCES];
	int mp_irq_entries;
	int num_processors;
	int boot_cpu_physical_apicid;

	u8_t bios_cpu_apicid[NR_CPUS];

	int mp_bus_id_to_type [MAX_MP_BUSSES];
	int mp_bus_id_to_pci_bus [MAX_MP_BUSSES];
	int mp_current_pci_id;
	int mpc_record;
public:
	SMP();

	bool found_config;
	bool pic_mode;
	u32_t mp_lapic_addr;

	int nr_ioapics;
};
#endif

