/*
  kernel/drivers/apic.h
  Copyright (C) 2008 Sergey Gridassov
*/

#ifndef __APIC_H
#define __APIC_H

#include <types.h>

#define IA32_APIC_BASE	0x0000001b
// APIC Base memory-mapped constant
#define APIC_MSR_BASE_M         0xfffff000  /* APIC MSR memory base address mask    */
#define APIC_MSR_GENA           0x00000800  /* APIC MSR Global Enable flag          */
#define APIC_MSR_BSP            0x00000100  /* APIC MSR BootStrap Processor flag    */

// APIC Base memory-mapped constant
#define APIC_BASE               0xffe00000  /* APIC Base Address                    */

// APIC Offsets for DWORD * base
#define APIC_DWREG_ID               0x0008  /* Local APIC Identifier                */
#define APIC_DWREG_VER              0x000c  /* Local APIC Version                   */
#define APIC_DWREG_TPR              0x0020  /* Task Priority Register               */
#define APIC_DWREG_APR              0x0024  /* Arbitration Priority Register        */
#define APIC_DWREG_PPR              0x0028  /* Processor Priority Register          */
#define APIC_DWREG_EOI              0x002c  /* EOI Register                         */
#define APIC_DWREG_LDR              0x0034  /* Logical Destination Register         */
#define APIC_DWREG_DFR              0x0038  /* Destination Format Register          */
#define APIC_DWREG_SVR              0x003c  /* Spurious Interrupt Vector Register   */
#define APIC_DWREG_ISR              0x0040  /* In-Service Register                  */
#define APIC_DWREG_TMR              0x0060  /* Trigger Mode Register                */
#define APIC_DWREG_IRQ              0x0080  /* Interrupt Request Register           */
#define APIC_DWREG_ESR              0x00a0  /* Error Status Register                */
#define APIC_DWREG_ICR0             0x00c0  /* Interrupt Command Register 0         */
#define APIC_DWREG_ICR1             0x00c4  /* Interrupt Command Register 1         */
#define APIC_DWREG_LVT_TMR          0x00c8  /* LVT Timer Register                   */
#define APIC_DWREG_LVT_THERM        0x00cc  /* LVT Thermal Sensor Register          */
#define APIC_DWREG_LVT_PERF         0x00d0  /* LVT Perfomance Monitoring Counters   */
#define APIC_DWREG_LVT_LINT0        0x00d4  /* LVT LINT0 Register                   */
#define APIC_DWREG_LVT_LINT1        0x00d8  /* LVT LINT1 Register                   */
#define APIC_DWREG_LVT_ERR          0x00dc  /* LVT Error Register                   */
#define APIC_DWREG_TMR_INIT         0x00e0  /* Timer Initial Count Register         */
#define APIC_DWREG_TMR_CURR         0x00e4  /* Timer Current Count Register         */
#define APIC_DWREG_TMR_DIVIDE       0x00f8  /* Timer Divide Configuration Register  */

// MP Structure signature
#define MP_SIGNATURE            0x5f504d5f  /* MP Structure Signature "_MP_"        */
#define MPC_SIGNATURE           0x504d4350  /* MP Table Header Signature "PCMP"     */

// MP Entry Type
#define MP_ENTRY_PROCESSOR               0  /* Processor                            */
#define MP_ENTRY_BUS                     1  /* Bus                                  */
#define MP_ENTRY_IO_APIC                 2  /* IO APIC                              */
#define MP_ENTRY_IO_INT                  3  /* IO Interrupt Assignment              */
#define MP_ENTRY_LOCAL_INT               4  /* Local Interrupt Assignment           */


class APIC {
private:
	u32_t *apic;
public:
	APIC();
	void setVirtualWire();
};

#endif
