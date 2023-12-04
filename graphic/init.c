#include <stdio.h>
#include <memory.h>
#include <graphic.h>
#include <debug.h>

#define MODULE "graphic"
#define MODULE_DEBUG 1

void graphic_init(void)
{
	gpu_init((struct gpu_info *)phys_to_virt(0x0));

	gpu_fill_rect(0, 0, vram_width() - 1, vram_height() - 1, Ivory);
	gpu_fill_rect(0, vram_height() - 51, vram_width() - 1, vram_height() - 1, LightGreen);

	pr_info("init graphic success");
}
