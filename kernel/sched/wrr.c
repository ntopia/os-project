#include "sched.h"
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/pid.h>
#include <linux/cred.h>

#include <uapi/asm-generic/errno-base.h>

#define WRR_WEIGHT_MIN	1
#define WRR_WEIGHT_MAX	20


struct task_struct *find_task_by_pid(pid_t pid)
{
	struct task_struct *task = NULL;
	struct pid *p_struct = NULL;
	
	if (pid == 0)
		return current;

	p_struct = find_get_pid(pid);
	if (!p_struct)
		return NULL;
	
	task = get_pid_task(p_struct, PIDTYPE_PID);
	return task;
}

void init_wrr_rq(struct wrr_rq *wrr_rq, struct rq *rq)
{
	raw_spin_lock_init(&wrr_rq->wrr_lock);
}


const struct sched_class wrr_sched_class = {
	.next			= &fair_sched_class,
//	.enqueue_task		= enqueue_task_wrr,
//	.dequeue_task		= dequeue_task_wrr,
//	.yield_task		= yield_task_wrr,

//	.check_preempt_curr	= check_preempt_curr_wrr,

//	.pick_next_task		= pick_next_task_wrr,
//	.put_prev_task		= put_prev_task_wrr,

#ifdef CONFIG_SMP
//	.select_task_rq		= select_task_rq_wrr,

//	.set_cpus_allowed	= set_cpus_allowed_wrr,
//	.rq_online		= rq_online_wrr,
//	.rq_offline		= rq_offline_wrr,
//	.pre_schedule		= pre_schedule_wrr,
//	.post_schedule		= post_schedule_wrr,
//	.task_woken		= task_woken_wrr,
//	.switched_from		= switched_from_wrr,
#endif

//	.set_curr_task		= set_curr_task_wrr,
//	.task_tick		= task_tick_wrr,

//	.get_rr_interval	= get_rr_interval_wrr,

//	.prio_changed		= prio_changed_wrr,
//	.switched_to		= switched_to_wrr,
};


/*
 * Set the SCHED_WRR weight of process, as identified by 'pid'.
 * If 'pid' is 0, set the weight for the calling process.
 * System call number 380.
 */
SYSCALL_DEFINE2(sched_setweight, pid_t, pid, int, weight)
{
	struct task_struct *task;
	if (weight < WRR_WEIGHT_MIN || weight > WRR_WEIGHT_MAX)
		return -EINVAL;

	task = find_task_by_pid(pid);
	if (task == NULL)
		return -EINVAL;

	if (task->policy != SCHED_WRR) 
		return -EINVAL;

	if (!current_uid())
		task->wrr.weight = weight;
	else if (current_uid() == task->cred->uid && task->wrr.weight > weight)
		task->wrr.weight = weight;
	else
		return -EACCES;
	return 0;
}

/*
 * Obtain the SCHED_WRR weight of a process as identified by 'pid'.
 * If 'pid' is 0, return the weight of the calling process.
 * System call number 381.
 */
SYSCALL_DEFINE1(sched_getweight, pid_t, pid)
{
	struct task_struct *task;
	task = find_task_by_pid(pid);
	if (task == NULL)
		return -EINVAL;

	if (task->policy != SCHED_WRR) 
		return -EINVAL;
	return task->wrr.weight;
}

