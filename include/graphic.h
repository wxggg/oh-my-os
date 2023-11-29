#include <types.h>
#include <color.h>

struct gpu_info {
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

void graphic_init(void);
void gpu_init(struct gpu_info *info);

void gpu_set_pixel(u32 x, u32 y, struct color c);
void gpu_draw_line(u32 x1, u32 x2, u32 y, struct color c);
void gpu_fill_rect(u32 x1, u32 y1, u32 x2, u32 y2, struct color c);

extern struct gpu_info g_gpu;

static inline u32 vram_width(void)
{
	return g_gpu.width;
}

static inline  u32 vram_height(void)
{
	return g_gpu.height;
}
