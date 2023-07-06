#pragma once

#include <types.h>

void dump_stack(void);
void dump_trapstack(uint32_t ebp, uint32_t eip);
void debug_init(void);
void monitor(void);
