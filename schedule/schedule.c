#include <schedule.h>
#include <kmalloc.h>
#include <memory.h>
#include <kernel.h>
#include <register.h>
#include <stdio.h>
#include <debug.h>
#include <smp.h>
#include <lock.h>

#define MODULE "schedule"
#define MODULE_DEBUG 1

extern char bootstack[];

static struct process init_proc;

struct run_queue rqs[MAX_CPU];
struct thread *current_threads[MAX_CPU];

#define this_rq ((struct run_queue *)&rqs[cpu_id()])

static uint32_t g_thread_id = 0;

static spinlock_t sched_lock[MAX_CPU];

void thread_entry(void);
void run_entrys(struct trapframe *tf);

void context_switch(struct thread_context *from, struct thread_context *to);

static void run_entry(void)
{
	run_entrys(current->tf);
}

void thread_exit(int err)
{
	current->state = THREAD_EXIT;
	schedule();
}

static int thread_state_read(struct file *file, string *s)
{
	struct thread *t = file->priv;

	ksappend_int(s, t->state);
	return 0;
}

static struct file_operations thread_state_fops = {
	.read = thread_state_read,
};

static int create_thread_procfs(struct thread *t)
{
	int ret;
	string *s;

	s = ksalloc();
	if (!s)
		return -ENOMEM;

	ksappend_int(s, t->tid);

	ret = create_directory(s->str, proc, &t->dir);
	if (ret)
		goto err_free_str;

	create_file("state", &thread_state_fops, t->dir, t, &t->f_state);
	return 0;

err_free_str:
	ksfree(s);
	return ret;
}

struct thread *thread_run(int (*fn)(void *), void *arg, int cpu)
{
	int ret;
	struct thread *t;

	if (cpu < 0 || cpu >= MAX_CPU)
		cpu = cpu_id();

	t = kmalloc(sizeof(*t));
	if (!t)
		return NULL;

	t->kstack = (uintptr_t)kmalloc(KERNEL_STACK_SIZE);
	if (!t->kstack)
		goto err_free_thread;

	t->tf = (struct trapframe *)(t->kstack + KERNEL_STACK_SIZE) - 1;
	t->tf->cs = KERNEL_CS;
	t->tf->ds = KERNEL_DS;
	t->tf->es = KERNEL_DS;
	t->tf->ss = KERNEL_DS;
	t->tf->reg.ebp = t->kstack;
	t->tf->reg.ebx = (uint32_t)fn;
	t->tf->reg.edx = (uint32_t)arg;
	t->tf->reg.esp = 0;
	t->tf->reg.eax = 0;
	t->tf->eip = (uint32_t)thread_entry;
	t->tf->eflags |= FL_IF;

	t->context.esp = (uintptr_t)t->tf;
	t->context.eip = (uintptr_t)run_entry;
	t->context.ebp = (uintptr_t)t->kstack;

	t->proc = current->proc;
	t->tid = g_thread_id++;
	t->state = THREAD_RUNNABLE;

	spin_lock(&sched_lock[cpu]);
	list_insert(&current_threads[cpu]->proc->thread_group, &t->node);
	list_insert(&rqs[cpu].head, &t->sched_node);
	spin_unlock(&sched_lock[cpu]);

	pr_debug("create thread-", dec(t->tid), " on cpu-", dec(cpu));

	ret = create_thread_procfs(t);
	if (ret)
		goto err_free_kstack;

	return t;

err_free_kstack:
	kfree((void *)t->kstack);
err_free_thread:
	kfree(t);
	return NULL;
}

void schedule(void)
{
	struct list_node *node;
	struct thread *prev = current, *next;
	struct thread_context context;
	int cpu = cpu_id();

	if (list_empty(&this_rq->head))
		return;

	node = list_next(&this_rq->head);
	next = container_of(node, struct thread, sched_node);

	if (prev == next)
		return;

	/* pr_debug("schedule: ", dec(current->tid), " => ", dec(next->tid)); */

	spin_lock(&sched_lock[cpu]);
	list_remove(&next->sched_node);
	spin_unlock(&sched_lock[cpu]);

	current = next;

	if (prev->state != THREAD_EXIT) {
		spin_lock(&sched_lock[cpu]);
		list_insert_tail(&this_rq->head, &prev->sched_node);
		spin_unlock(&sched_lock[cpu]);
		prev->state = THREAD_RUNNABLE;
		next->state = THREAD_RUNNING;
		context_switch(&prev->context, &next->context);
	} else {
		spin_lock(&sched_lock[cpu]);
		list_remove(&prev->node);
		spin_unlock(&sched_lock[cpu]);
		remove_directory(prev->dir);
		kfree((void *)prev->kstack);
		kfree(prev);
		context_switch(&context, &next->context);
	}
}

int schedule_init(int cpu)
{
	struct run_queue *rq = &rqs[cpu];
	struct thread *idle;

	spinlock_init(&sched_lock[cpu]);

	pr_info("init schedule on cpu-", dec(cpu));

	if (cpu == 0)
		list_init(&init_proc.thread_group);

	list_init(&rq->head);

	idle = kmalloc(sizeof(*idle));
	if (!idle)
		return -ENOMEM;

	idle->tid = g_thread_id++;
	idle->state = THREAD_RUNNING;
	idle->proc = &init_proc;
	idle->kstack = (uint32_t)bootstack;
	idle->tf = (struct trapframe *)(idle->kstack + KERNEL_STACK_SIZE * cpu) - 1;

	pr_debug("create idle thread-", dec(idle->tid));

	current_threads[cpu] = idle;

	list_insert(&init_proc.thread_group, &idle->node);

	create_thread_procfs(idle);

	return 0;
}
