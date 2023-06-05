#include <process.h>

static struct process __init_proc;

struct process *current_task(void)
{
	return &__init_proc;
}

int process_init(void)
{
	return 0;
}
