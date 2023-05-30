#include <stdio.h>
#include <memory.h>

struct color {
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

static struct color bgcolor = { 0x1E, 0x22, 0x29 };
static struct color maincolor = { 0x17, 0xA8, 0x88 };

struct graphic_info {
	/* hard coded */
	uint8_t 	cyls;
	uint8_t 	leds;
	uint8_t 	vmode;
	uint8_t 	reserve1;
	uint16_t 	width;
	uint16_t 	height;
	uint8_t 	pixelbits;
	uint8_t 	mem_model;
	uint16_t 	reserve2;
	uintptr_t 	vram;

	/* extend value */
	uint8_t		pixelbytes;
};

static inline void set_pixel(struct graphic_info *info, int16_t x, int16_t y,
		int8_t r, int8_t g, int8_t b)
{
	uint8_t *p = (uint8_t *) (info->vram + info->pixelbytes * info->width * y
			+ info->pixelbytes * x);
	*p = b;
	*(p + 1) = g;
	* (p + 2) = r;
}

static void fill_area_condition(struct graphic_info *info , int16_t w,
		int16_t h, int8_t r, int8_t g, int8_t b,
		bool (*condition)(int16_t, int16_t))
{
	for (int16_t x = 0; x < w; x++) {
		for (int16_t y = 0; y < h; y++) {
			if (!condition || condition(x, y)) {
				set_pixel(info, x, y, r, g, b);
			}
		}
	}
}

static bool circle_condition(int16_t x, int16_t y)
{
	if (x % 32 == 0 || y % 32 == 0) return true;
	return false;
}

void graphic_init(void)
{
	struct graphic_info *info = (struct graphic_info *) phys_to_virt(0x0);
	info->pixelbytes = info->pixelbits >> 3;
	unsigned long size = info->width * info->height * info->pixelbytes;

	pr_info("init graphic:");
	pr_info("\tscreen width:", dec(info->width));
	pr_info("\tscreen height:", dec(info->height));
	pr_info("\tpixel bits:", dec(info->pixelbits));
	pr_info("\tvram:", hex(info->vram));

	kernel_map(info->vram, info->vram, size, PTE_W);

	fill_area_condition(info, info->width, info->height, bgcolor.r,
			bgcolor.g, bgcolor.b, NULL);

	fill_area_condition(info, info->width, info->height, maincolor.r,
			maincolor.g, maincolor.b, circle_condition);


	pr_info("init graphic success");
}
