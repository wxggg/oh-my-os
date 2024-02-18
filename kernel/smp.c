#include <assert.h>
#include <memory.h>
#include <string.h>
#include <stdio.h>
#include <smp.h>
#include <fs.h>
#include <irq.h>

#define MODULE "smp"
#define MODULE_DEBUG 0

struct cpu {
	int id;
	struct mp_processor *processor;
};

struct cpu cpus[8];

u32 nr_cpu;

extern volatile u8 ioapic_id;

static int list_cpu(struct file *file, vector *v)
{
	struct cpu *cpu;
	struct mp_processor *processor;
	int i;

	for (i = 0; i < nr_cpu; i++) {
		cpu = &cpus[i];

		processor = cpu->processor;

		printk("cpu-", dec(cpu->id),
		       " lapic_id:", dec(processor->lapic_id),
		       " en:", dec(processor->cpu_en),
		       " bootstrap:", dec(processor->cpu_bp),
		       " stepping:", dec(processor->cpu_stepping),
		       " model:", dec(processor->cpu_model),
		       " family:", dec(processor->cpu_family), "\n");
	}

	return 0;
}

/* search mp in rom space */
struct mp *mp_search()
{
	struct mp *mp;
	u32 virt = phys_to_virt(ROM_BASE);
	u32 offset;

	assert(sizeof(struct mp) == ROM_ENTITY_SIZE);

	for (offset = 0; offset < ROM_SIZE; offset += ROM_ENTITY_SIZE) {
		mp = (void *)(virt + offset);

		if (!memcmp(mp->signature, "_MP_", 4))
			return mp;
	}

	return NULL;
}

int scan_mp_entry(u8 *start, u8 *end)
{
	u8 *p = start;
	struct mp_processor *processor;
	struct mp_bus *bus;
	struct mp_ioapic *ioapic;
	struct mp_iointr *iointr;
	struct mp_lintr *lintr;

	nr_cpu = 0;

	for (p = start; p < end;) {
		switch (*p) {
		case MP_ENTRY_TYPE_PROCESSOR:
			processor = (struct mp_processor *)p;
			pr_info("find processor lapic_id:",
				dec(processor->lapic_id),
				" en:", dec(processor->cpu_en),
				" bootstrap:", dec(processor->cpu_bp),
				" stepping:", dec(processor->cpu_stepping),
				" model:", dec(processor->cpu_model),
				" family:", dec(processor->cpu_family));
			cpus[nr_cpu].id = nr_cpu;
			cpus[nr_cpu].processor = processor;
			nr_cpu++;
			p += sizeof(*processor);
			break;
		case MP_ENTRY_TYPE_BUS:
			bus = (struct mp_bus *)p;
			pr_info("find bus-", dec(bus->bus_id), " ",
				bus->bus_type_string);
			p += sizeof(*bus);
			break;
		case MP_ENTRY_TYPE_IOAPIC:
			ioapic = (struct mp_ioapic *)p;
			ioapic_id = ioapic->ioapic_id;
			pr_info("find ioapic-", dec(ioapic->ioapic_id));
			p += sizeof(*ioapic);
			break;
		case MP_ENTRY_TYPE_IOINTR:
			iointr = (struct mp_iointr *)p;
			pr_info("find iointr interrupt:",
				dec(iointr->interrupt));
			p += sizeof(*iointr);
			break;
		case MP_ENTRY_TYPE_LINTR:
			lintr = (struct mp_lintr *)p;
			pr_info("find lintr interrupt:",
				dec(iointr->interrupt));
			p += sizeof(*lintr);
			break;
		default:
			pr_err("unknown mp entry type ", dec(*p));
			return -ENODEV;
		}
	}

	return 0;
}

int smp_init(void)
{
	struct mp *mp;
	struct mp_config *config;
	string *s = ksalloc();

	mp = mp_search();
	if (!mp) {
		pr_info("failed mp failed");
		return -ENOENT;
	}

	pr_info("find mp on:", hex(mp),
		" config phys addr:", hex(mp->mp_confg_addr),
		" version:", dec(mp->version), " length:", dec(mp->length),
		" imcrp:", dec(mp->imcrp));

	config = (void *)phys_to_virt(mp->mp_confg_addr);
	if (memcmp(config->signature, "PCMP", 4)) {
		pr_info("invalid mp config signature.");
		return -EINVAL;
	}

	ksappend_str(s, " oem_id:");
	ksappend_strn(s, (char *)config->oem_id, 8);
	ksappend_str(s, " product_id:");
	ksappend_strn(s, (char *)config->product_id, 12);

	pr_info("mp config: length:", dec(config->length),
		" version:", dec(config->version), s->str,
		" lapic_addr:", hex(config->lapic_addr));

	lapic = (void *)config->lapic_addr;

	scan_mp_entry((u8 *)config + sizeof(*config),
		      (u8 *)config + config->length);

	if (mp->imcrp) {
		outb(0x22, 0x70);
		outb(0x23, inb(0x23) | 1);
	}

	return 0;
}

struct file_operations list_cpu_ops = {
	.exec = list_cpu,
};

int smp_init_late(void)
{
	struct file *file;

	binfs_create_file("lscpu", &list_cpu_ops, NULL, &file);

	return 0;
}
