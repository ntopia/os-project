/*
 *	linux/kernel/ptree.c
 *
 *	snu os project 1
 *	team 10
 */

#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/cred.h>
#include <linux/string.h>

#include <linux/prinfo.h>

#include <uapi/asm-generic/errno-base.h>

struct task_struct* get_first_child(struct task_struct *task)
{
	return list_first_entry_or_null(&task->children, struct task_struct, sibling);
}

struct task_struct* get_next_sibling(struct task_struct *task)
{
	if (list_is_last(&task->sibling, &task->parent->children))
		return NULL;
	return list_first_entry_or_null(&task->sibling, struct task_struct, sibling);
}

struct task_struct* get_next_struct(struct task_struct *task)
{
	/*
	 * return value:
	 *   (if has) child -> sibling -> parent's sibling -> ...
	 *   (if not) null
	 */
	
	struct task_struct *next = get_first_child(task);
	if (next)
		return next;
	while (task->pid)
	{
		next = get_next_sibling(task);
		if (next && next->pid)
			return next;
		if (next && !next->pid)
			return NULL;
		task = task->parent;
	}
	return NULL;
}

pid_t get_pid_from_task(struct task_struct *task)
{
	if (task == NULL)
		return 0;
	return task->pid;
}

long do_ptree(struct prinfo *kbuf, int *nr_value)
{
	/*
	 * push prinfo struct to kbuf in Depth First Search order.
	 * modify nr_value, 
	 * if total number of entries is smaller than that.
	 * 
	 * return value: total number of entries on success, or the error code
	 * 	-EFAULT: if buf are outside the accessible address space
	 */
	long entry_num = 0;
	struct task_struct *curr_task;

	printk(KERN_ALERT"**** do_ptree called\n");
	read_lock(&tasklist_lock);
	printk(KERN_ALERT"**** read locked\n");
	curr_task = &init_task;
	while (curr_task)
	{
		printk(KERN_ALERT"**** [%ld] %s: %d\n", entry_num, curr_task->comm, curr_task->pid);
		if (entry_num < *nr_value)
		{
			kbuf[entry_num].state = curr_task->state;
			kbuf[entry_num].pid = curr_task->pid;
			kbuf[entry_num].parent_pid = curr_task->real_parent->pid;
			kbuf[entry_num].first_child_pid = get_pid_from_task(get_first_child(curr_task));
			printk(KERN_ALERT"**** child's pid is [%d]\n", kbuf[entry_num].first_child_pid);
			kbuf[entry_num].state = curr_task->state;
			kbuf[entry_num].next_sibling_pid = get_pid_from_task(get_next_sibling(curr_task));
			printk(KERN_ALERT"**** sibling's pid is [%d]\n", kbuf[entry_num].next_sibling_pid);
			kbuf[entry_num].uid = task_uid(curr_task);
			strncpy(kbuf[entry_num].comm, curr_task->comm, 64);
		}
		curr_task = get_next_struct(curr_task);
		entry_num++;
	}
	read_unlock(&tasklist_lock);
	printk(KERN_ALERT"**** read unlocked\n");
	
	if (*nr_value > entry_num) *nr_value = entry_num;
	return entry_num;
}


/*
 * entry point for ptree
 */

SYSCALL_DEFINE2(ptree, struct prinfo __user *, buf, int __user *, nr)
{
	/*
	 * prinfo: process information struct defined in include/linux/prinfo.h
	 * buf: buffer for the process data
	 * nr: size of this buffer
	 * return value: total number of entries on success, or the error code
	 * 	-EINVAL: if buf or nr are null,
	 * 	or if the number of entries is less than 1
	 * 	-EFAULT: if buf or nr are outside the accessible address space
	 */
	int nr_value;
	struct prinfo *kbuf;
	long result = 0;

	printk(KERN_ALERT"**** ptree called\n");
	if (buf == NULL || nr == NULL)
		return -EINVAL;
	if (get_user(nr_value, nr))
		return -EFAULT;
	if (nr_value <= 1)
		return -EINVAL;

	kbuf = (struct prinfo*)kmalloc(nr_value * sizeof(struct prinfo), GFP_KERNEL);
	printk(KERN_ALERT"**** kbuf allocated\n");
	if (kbuf == NULL)
		return -ENOMEM;
	result = do_ptree(kbuf, &nr_value);
	printk(KERN_ALERT"**** do_ptree end\n");

	if (copy_to_user(buf, kbuf, nr_value*sizeof(struct prinfo)))
		return -EFAULT;
	if (put_user(nr_value, nr))
		return -EFAULT;
	return result;
}
