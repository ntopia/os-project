#include "sched.h"
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/pid.h>
#include <linux/cred.h>

#include <uapi/asm-generic/errno-base.h>

#define WRR_WEIGHT_MIN		1
#define WRR_WEIGHT_MAX		20
#define WRR_WEIGHT_DEFAULT	10
#define WRR_BASE_TIME_SLICE	10


void init_wrr_rq(struct wrr_rq *wrr_rq, struct rq *rq)
{
	raw_spin_lock_init(&wrr_rq->wrr_lock);

	INIT_LIST_HEAD(&wrr_rq->run_list);
	wrr_rq->wrr_nr_running = 0;
	wrr_rq->weight_sum = 0;
}

/*
 * Get owner(task_struct) of given sched_wrr_entity
 */
static struct task_struct *wrr_task_of(struct sched_wrr_entity *wrr_se)
{
	return container_of(wrr_se, struct task_struct, wrr);
}

/*
 * Check if the weight is valid for wrr
 */
static bool is_valid_wrr_weight(unsigned int weight)
{
	return (WRR_WEIGHT_MIN <= weight && weight <= WRR_WEIGHT_MAX);
}

/*
 * Calculate time slice for wrr task
 */
static unsigned int calc_wrr_time_slice(unsigned int weight)
{
	return WRR_BASE_TIME_SLICE * weight;
}

/*
 * Initialize wrr entity for newly enqueued task
 */
static void init_wrr_task(struct task_struct *p)
{
	struct sched_wrr_entity *wrr_entity;

	if (p == NULL)
		return;

	wrr_entity = &p->wrr;
	if (!is_valid_wrr_weight(wrr_entity->weight)) {
		/* maybe newly created task */
		wrr_entity->weight = WRR_WEIGHT_DEFAULT;
	}
	else {
		/* requeued task */
	}
	wrr_entity->time_slice = calc_wrr_time_slice(wrr_entity->weight);
}


/*
 * Adding/removing a task to/from our data structure
 */
static void enqueue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
	struct wrr_rq *wrr_rq = &rq->wrr;
	struct sched_wrr_entity *wrr_entity = &p->wrr;

	init_wrr_task(p);

	list_add_tail(&wrr_entity->run_list, &wrr_rq->run_list);
	wrr_rq->wrr_nr_running++;
	wrr_rq->weight_sum += wrr_entity->weight;
}

static void dequeue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
	struct wrr_rq *wrr_rq = &rq->wrr;
	struct sched_wrr_entity *wrr_entity = &p->wrr;

	list_del_init(&wrr_entity->run_list);
	wrr_rq->wrr_nr_running--;
	wrr_rq->weight_sum -= wrr_entity->weight;
}

/*
 * If head == 0, p moves the front of wrr_rq.
 * Else, p moves the end of wrr_rq.
 */
static void requeue_task_wrr(struct rq *rq, struct task_struct *p, int head)
{
	struct wrr_rq *wrr_rq = &rq->wrr;
	struct sched_wrr_entity *wrr_se = &p->wrr;

	if (head)
		list_move(&wrr_se->run_list, &wrr_rq->run_list);
	else
		list_move_tail(&wrr_se->run_list, &wrr_rq->run_list);
}

/*
 * This function chooses the most appropriate task eligible to run next.
 * It just returns the front of wrr_rq.
 */
static struct task_struct *pick_next_task_wrr(struct rq *rq)
{
	struct wrr_rq *wrr_rq = &rq->wrr;
	struct sched_wrr_entity *wrr_entity;

	if (list_empty(&wrr_rq->run_list))
		return NULL;

	wrr_entity = container_of(wrr_rq->run_list.next, struct sched_wrr_entity, run_list);
	return wrr_task_of(wrr_entity);
}

/*
 * A scheduling class hook that informs the task's class
 * that the given task is about to be switched out of the CPU
 */
static void put_prev_task_wrr(struct rq *rq, struct task_struct *p)
{
	/*
	 * I think that we don't need to do something in this func
	 */
}

/*
 * This is called from time tick funcs (scheduler_tick(), hrtick()).
 * Similar to task_tick_rt().
 * This is called with HZ frequency.
 */
static void task_tick_wrr(struct rq *rq, struct task_struct *p, int queued)
{
	struct sched_wrr_entity *wrr_se = &p->wrr;

	if (p->policy != SCHED_WRR)
		return;

	if (--p->wrr.time_slice)
		return;

	/*
	 * Task's time slice is done
	 */

	p->wrr.time_slice = calc_wrr_time_slice(p->wrr.weight);

	/*
	 * Requeue to the end of queue
	 * if we are the only element on the queue
	 */
	if (wrr_se->run_list.prev != wrr_se->run_list.next) {
		requeue_task_wrr(rq, p, 0);
		set_tsk_need_resched(p);
	}
}


const struct sched_class wrr_sched_class = {
	.next			= &fair_sched_class,
	.enqueue_task		= enqueue_task_wrr,
	.dequeue_task		= dequeue_task_wrr,
//	.yield_task		= yield_task_wrr,

//	.check_preempt_curr	= check_preempt_curr_wrr,

	.pick_next_task		= pick_next_task_wrr,
	.put_prev_task		= put_prev_task_wrr,

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
	.task_tick		= task_tick_wrr,

//	.get_rr_interval	= get_rr_interval_wrr,

//	.prio_changed		= prio_changed_wrr,
//	.switched_to		= switched_to_wrr,
};


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

