#pragma once
#include <mm.h>
#include <list.h>
#include <irq.h>
#include <string.h>
#include <fs.h>
#include <smp.h>

struct thread_context {
	uint32_t eip;
	uint32_t esp;
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
	uint32_t esi;
	uint32_t edi;
	uint32_t ebp;
};

enum thread_state {
	THREAD_INACTIVE = 0,
	THREAD_SLEEPING,
	THREAD_RUNNABLE,
	THREAD_RUNNING,
	THREAD_EXIT
};

struct thread {
	u32 tid;
	string *s;
	enum thread_state state;
	struct process *proc;
	uintptr_t kstack;
	struct trapframe *tf;
	struct thread_context context;
	struct list_node node;
	struct list_node sched_node;

	struct directory *dir;
	struct file *f_state;
	struct file *f_context;
};

struct process {
	u32 pid;
	struct mm_context *mm;
	struct list_node thread_group;
};

struct run_queue {
	struct list_node head;
};

int schedule_init(int cpu);
void schedule(void);

struct thread *thread_run(int (*fn)(void *), void *arg, int cpu);
void thread_exit(int err);
void thread_sleep(struct thread *thread);
void thread_wakeup(struct thread *thread);

extern struct thread *current_threads[MAX_CPU];
#define current current_threads[cpu_id()]
