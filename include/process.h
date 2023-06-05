#pragma once
#include <mm.h>

struct process {
	struct mm_context *mm;
};

struct process *current_task(void);
#define current current_task()

int process_init(void);
