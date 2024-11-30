#pragma once

#include <types.h>

#define ROM_BASE 0xF0000
#define ROM_SIZE 0x10000
#define ROM_ENTITY_SIZE 0x10

struct mp {
	/* _MP_ */
	u8 signature[4];

	/* The address of the beginning of the MP configuration table.
	 * All zeros if the MP configuration table does not exist. */
	u32 mp_confg_addr;

	/* The length of the floating pointer structure table in paragraph
	 * (16-byte) units. The structure is 16 bytes or 1 paragraph long;
	 * so this field contains 01h. */
	u8 length;
	u8 version;
	u8 checksum;

	/* Bits 0-7: MP System Configuration Type.  When these bits are all
	 * zeros, the MP configuration table is present. When nonzero, the
	 * value indicates which default configuration (as defined in Chapter 5)
	 * is implemented by the system. */
	u8 type;

	struct {
		u8 reserve1 : 7;

		/* When the IMCR presence bit is set, the IMCR is present
		 * and PIC Mode is implemented; otherwise, Virtual Wire Mode
		 * is implemented. */
		u8 imcrp : 1;
	};

	u8 reserve2[3];
};

struct mp_config {
	/* PCMP */
	u8 signature[4];

	u16 length;
	u8 version;
	u8 checksum;

	/* A string that identifies the manufacturer of the system hardware. */
	u8 oem_id[8];
	/* A string that identifies the product family. */
	u8 product_id[12];
	u32 oem_table_addr;
	u16 oem_table_size;

	u16 entry_count;
	/* The base address by which each processor accesses its local APIC. */
	u32 lapic_addr;
	u16 extend_table_length;
	u8 extend_table_checksum;
};

#define MP_ENTRY_TYPE_PROCESSOR 0
#define MP_ENTRY_TYPE_BUS 1
#define MP_ENTRY_TYPE_IOAPIC 2
#define MP_ENTRY_TYPE_IOINTR 3
#define MP_ENTRY_TYPE_LINTR 4

struct mp_processor {
	u8 type;
	u8 lapic_id;
	u8 lapic_version;
	/* If zero, this processor is unusable, and the operating system should
	 * not attempt to access this processor */
	union {
		u8 flags;
		struct {
			u8 cpu_en : 1;
			u8 cpu_bp : 1;
		};
	};

	union {
		u32 signature;
		struct {
			u8 cpu_stepping : 4;
			u8 cpu_model : 4;
			u8 cpu_family : 4;
		};
	};

	u32 feature;
	u32 reserve1;
	u32 reserve2;
};

struct mp_bus {
	u8 type;
	/* An integer that identifies the bus entry. The BIOS assigns
	 * identifiers sequentially, starting at zero. */
	u8 bus_id;
	/* CBUS		Corollary CBus
	 * CBUSII	Corollary CBUS II
	 * EISA		Extended ISA
	 * FUTURE	IEEE FutureBus
	 * INTERN	Internal bus
	 * ISA		Industry Standard Architecture
	 * MBI		Multibus I
	 * MBII		Multibus II
	 * MCA		Micro Channel Architecture
	 * MPI		MPI
	 * MPSA		MPSA
	 * NUBUS	Apple Macintosh NuBus
	 * PCI		Peripheral Component Interconnect
	 * PCMCIA	PC Memory Card International Assoc.
	 * TC		DEC TurboChannel
	 * VL		VESA Local Bus
	 * VME		VMEbus
	 * XPRESS	Express System Bus
	 */
	u8 bus_type_string[6];
};

struct mp_ioapic {
	u8 type;
	u8 ioapic_id;
	u8 ioapic_version;
	u8 ioapic_flags_en : 1;
	u8 reserve1 : 7;
	u32 ioapic_addr;
};

struct mp_iointr {
	u8 type;
	u8 interrupt;
	/* Polarity of APIC I/O input signals:
	 * 00 = Conforms to specifications of bus (for example,
	 * EISA is activelow for level-triggered interrupts)
	 * 01 = Active high
	 * 10 = Reserved
	 * 11 = Active low
	 * Must be 00 if the 82489DX is used. */
	u8 polarity : 2;
	/* Trigger mode of APIC I/O input signals:
	 * 00 = Conforms to specifications of bus (for example, ISA is edgetriggered)
	 * 01 = Edge-triggered
	 * 10 = Reserved
	 * 11 = Level-triggered */
	u8 trigger_mode : 2;
	u8 reserve1 : 4;
	u8 reserve2;
	u8 src_bus_id;
	u8 src_bus_irq;
	u8 dst_ioapic_id;
	u8 dst_ioapic_intin;
};

struct mp_lintr {
	u8 type;
	u8 interrupt;
	u8 polarity;
	u8 trigger_mode;
	u8 src_bus_id;
	u8 src_bus_irq;
	u8 dst_lapic_id;
	u8 dst_lapic_lintin;
};

struct mm_context;

int smp_init(struct mm_context *mm);
int smp_init_late(void);

struct cpu {
	int id;
	bool init;
	bool started;
	struct mp_processor *processor;
};

#define MAX_CPU 8

extern struct cpu cpus[MAX_CPU];
extern u32 nr_cpu;

u32 cpu_id();
struct cpu *this_cpu();
int cpu_up(u32 cpu);
