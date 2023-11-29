/* 
 * gpu.c - a virtual gpu
 */

#include <graphic.h>
#include <string.h>
#include <memory.h>
#include <assert.h>
#include <fs.h>

struct gpu_info g_gpu;

static inline void assert_point(u32 x, u32 y)
{
	assert(x < g_gpu.width && y < g_gpu.height, "invalid x=", hex(x),
	       " y=", y);
}

static inline size_t pixel_bytes(void)
{
	return g_gpu.pixelbits >> 3;
}

static inline size_t vram_size(void)
{
	return g_gpu.height * g_gpu.width * pixel_bytes();
}

static inline color *vram_pixel(u32 x, u32 y)
{
	return (color *)(g_gpu.vram + (g_gpu.width * y + x) * pixel_bytes());
}

void gpu_set_pixel(u32 x, u32 y, color c)
{
	color *p;

	p = vram_pixel(x, y);
	*p = c;
}

void gpu_draw_line(u32 x1, u32 x2, u32 y, color c)
{
	u32 x;
	color *p;

	assert_point(x1, y);
	assert_point(x2, y);
	assert(x1 < x2, "invalid x:", range(x1, x2));

	p = vram_pixel(x1, y);

	for (x = x1; x <= x2; x++)
		*p++ = c;
}

void gpu_fill_rect(u32 x1, u32 y1, u32 x2, u32 y2, color c)
{
	color *p;
	u32 x, y;

	assert_point(x1, y1);
	assert_point(x2, y2);
	assert(x1 < x2, "invalid x:", range(x1, x2));
	assert(y1 < y2, "invalid y:", range(x1, x2));

	for (y = y1; y <= y2; y++) {
		p = vram_pixel(x1, y);

		for (x = x1; x <= x2; x++) {
			*p++ = c;
		}
	}
}

static int gpu_dump_read(struct file *file, string *s)
{
	ksappend_str(s, "screen:");
	ksappend_int(s, vram_width());
	ksappend_str(s, "x");
	ksappend_int(s, vram_height());
	ksappend_str(s, ", ");
	ksappend_str(s, "vram:");
	ksappend_str(s, hex(g_gpu.vram));

	return 0;
}

static struct file_operations gpu_dump_fops = {
	.read = gpu_dump_read,
};

void gpu_init(struct gpu_info *info)
{
	struct file *file;

	memcpy(&g_gpu, info, sizeof(g_gpu));

	pr_info("screen:", dec(vram_width()), "x", dec(vram_height()), ", ",
		"pixel bits:", dec(g_gpu.pixelbits), ", ",
		"vram:", hex(g_gpu.vram));

	kernel_map(g_gpu.vram, g_gpu.vram, vram_size(), PTE_W);

	create_file("gpu_dump", &gpu_dump_fops, sys, info, &file);
}
