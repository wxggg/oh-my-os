#include <schedule.h>
#include <kmalloc.h>
#include <memory.h>
#include <kernel.h>
#include <register.h>
#include <stdio.h>
#include <debug.h>

extern char bootstack[], bootstacktop[];

static struct process init_proc;
struct process *current_proc = &init_proc;

struct list_node run_list;

static uint32_t g_thread_id = 1;

struct thread idle_thread = {
	.tid = 0,
	.state = THREAD_RUNNING,
	.proc = &init_proc,
};

struct thread *current = &idle_thread;

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

static int create_therad_procfs(struct thread *t)
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

struct thread *thread_run(int (*fn)(void *), void *arg)
{
	int ret;
	struct thread *t;

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

	t->proc = current_proc;
	t->tid = g_thread_id++;
	t->state = THREAD_RUNNABLE;

	list_insert(&current_proc->thread_group, &t->node);
	list_insert(&run_list, &t->sched_node);

	ret = create_therad_procfs(t);
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

	if (list_empty(&run_list))
		return;

	node = list_next(&run_list);
	next = container_of(node, struct thread, sched_node);

	if (prev == next)
		return;

	/* pr_info("schedule: ", dec(current->tid), " => ", dec(next->tid)); */

	list_remove(&next->sched_node);

	current = next;

	if (prev->state != THREAD_EXIT) {
		list_insert_tail(&run_list, &prev->sched_node);
		prev->state = THREAD_RUNNABLE;
		next->state = THREAD_RUNNING;
		context_switch(&prev->context, &next->context);
	} else {
		list_remove(&prev->node);
		remove_directory(prev->dir);
		kfree((void *)prev->kstack);
		kfree(prev);
		context_switch(&context, &next->context);
	}
}

int schedule_init(void)
{
	struct trapframe *tf;

	list_init(&run_list);
	list_init(&current_proc->thread_group);

	tf = (struct trapframe *)(idle_thread.kstack + KERNEL_STACK_SIZE) - 1;

	idle_thread.kstack = (uint32_t)bootstack;
	idle_thread.tf = tf;
	list_insert(&current_proc->thread_group, &idle_thread.node);

	return 0;
}
