
#include <fs.h>
#include <kernel.h>
#include <stdio.h>
#include <assert.h>
#include <memory.h>

struct directory *current_dir;

static int do_devmem(struct file *file, vector *vec)
{
	int i, j, addr;
	u32 *p;

	if (vector_size(vec) < 2) {
		printk(__func__, ": invalid arguments ", dec(vector_size(vec)),
		       "\n");
		return -1;
	}

	if (hex_to_value(vector_at(vec, string *, 1), &addr)) {
		printk(__func__, ": invalid arguments ",
		       vector_at(vec, string *, 1)->str, "\n");
		return -1;
	}

	printk("read ", hex(addr), ":\n");

	for (i = 0; i < 16; i++) {
		for (j = 0; j < 4; j++) {
			p = (u32 *)phys_to_virt((addr + i * 16 + j * 4));
			printk(hex(*p), " ");
		}
		printk("\n");
	}

	printk("\n");

	return 0;
}

static struct file_operations devmem_fops = {
	.exec = do_devmem,
};

int mem_init(void)
{
	int ret;
	struct file *file;

	current_dir = root;

	ret = binfs_create_file("devmem", &devmem_fops, NULL, &file);
	if (ret)
		return ret;

	return 0;
}
