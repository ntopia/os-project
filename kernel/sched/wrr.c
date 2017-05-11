#include "sched.h"
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/pid.h>
#include <linux/cred.h>
#include <linux/interrupt.h>

#include <uapi/asm-generic/errno-base.h>
#include <linux/sched/wrr.h>

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
 * 1s ->	task_tick() is called HZ times
 * 1ms ->				HZ/1000 times
 * (weight*base)ms ->			weight*base*HZ/1000 times
 */
static unsigned int calc_wrr_time_slice(unsigned int weight)
{
	return weight * WRR_BASE_TIME_IN_MS * HZ / 1000;
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
	struct sched_wrr_entity *wrr_se;
	struct task_struct *p;
	struct wrr_rq *wrr_rq = &rq->wrr;

	if (list_empty(&wrr_rq->run_list))
		return NULL;

	wrr_se = container_of(wrr_rq->run_list.next, struct sched_wrr_entity, run_list);

	p = wrr_task_of(wrr_se);
	p->se.exec_start = rq->clock_task;

	return p;
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
 * This function is called
 * when a task changes its scheduling class or task group
 */
static void set_curr_task_wrr(struct rq *rq)
{
	struct task_struct *p = rq->curr;

	p->se.exec_start = rq->clock_task;
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

static void yield_task_wrr(struct rq *rq)
{
	requeue_task_wrr(rq, rq->curr, 0);
}

static void check_preempt_curr_wrr(struct rq *rq, struct task_struct *p, int flags)
{
	/*
	 * if class is different, it is handled at check_preempt_curr at core.c
	 * otherwise, all wrr task has the same priority
	 * hence we do nothing here
	 */
}


#ifdef CONFIG_SMP

static int select_task_rq_wrr(struct task_struct *p, int sd_flag, int flags)
{
	return task_cpu(p);
}

static void rq_online_wrr(struct rq *rq)
{
}

static void rq_offline_wrr(struct rq *rq)
{
}

static void pre_schedule_wrr(struct rq *rq, struct task_struct *prev)
{
}

static void post_schedule_wrr(struct rq *rq)
{
}

static void task_woken_wrr(struct rq *rq, struct task_struct *p)
{
}

static void switched_from_wrr(struct rq *rq, struct task_struct *p)
{
}

#endif /* CONFIG_SMP */


static unsigned int get_rr_interval_wrr(struct rq *rq, struct task_struct *task)
{
	return calc_wrr_time_slice(task->wrr.weight);
}

static void prio_changed_wrr(struct rq *rq, struct task_struct *p, int oldprio)
{
}

static void switched_to_wrr(struct rq *rq, struct task_struct *p)
{
}


#ifdef CONFIG_SMP

u64 last_wrr_rebalance_time = 0;

/*
 * This function triggers load-balancing. (Only cpu 0 can trigger)
 * First, it checks time if it should kick load-balance now.
 * If now is the time, then raise softirq to kick real load-balancing.
 * (Inspired by trigger_load_balance() in fair.c)
 */
void trigger_wrr_load_balance(struct rq *rq, int cpu)
{
	u64 delta, cur;
	if (cpu != 0)
		return;

	cur = sched_clock_cpu(cpu);
	delta = cur - last_wrr_rebalance_time;
	if (delta >= 1990000000) {	/* 2000ms */
		raise_softirq(SCHED_WRR_SOFTIRQ);
		last_wrr_rebalance_time = cur;
	}
}

/*
 * This function does real load-balancing.
 */
static void run_wrr_rebalance(struct softirq_action *h)
{
	int this_cpu = smp_processor_id();
	/* printk(KERN_ALERT "run_wrr_rebalance!!! %d\n", this_cpu); */

	/* Do load-balancing ! */
}

#endif



const struct sched_class wrr_sched_class = {
	.next			= &fair_sched_class,
	.enqueue_task		= enqueue_task_wrr,
	.dequeue_task		= dequeue_task_wrr,
	.yield_task		= yield_task_wrr,

	.check_preempt_curr	= check_preempt_curr_wrr,

	.pick_next_task		= pick_next_task_wrr,
	.put_prev_task		= put_prev_task_wrr,

#ifdef CONFIG_SMP
	.select_task_rq		= select_task_rq_wrr,

	.rq_online		= rq_online_wrr,
	.rq_offline		= rq_offline_wrr,
	.pre_schedule		= pre_schedule_wrr,
	.post_schedule		= post_schedule_wrr,
	.task_woken		= task_woken_wrr,
	.switched_from		= switched_from_wrr,
#endif

	.set_curr_task		= set_curr_task_wrr,
	.task_tick		= task_tick_wrr,

	.get_rr_interval	= get_rr_interval_wrr,

	.prio_changed		= prio_changed_wrr,
	.switched_to		= switched_to_wrr,
};

__init void init_sched_wrr_class(void)
{
#ifdef CONFIG_SMP
	open_softirq(SCHED_WRR_SOFTIRQ, run_wrr_rebalance);
#endif
}


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
	struct rq *rq;
	if (weight < WRR_WEIGHT_MIN || weight > WRR_WEIGHT_MAX)
		return -EINVAL;

	task = find_task_by_pid(pid);
	if (task == NULL)
		return -EINVAL;

	if (task->policy != SCHED_WRR) 
		return -EINVAL;

	if (!current_uid()
		|| current_uid() == task->cred->uid && task->wrr.weight > weight) {
		if (task->on_rq == TASK_ON_RQ_QUEUED) {
			rq = task_rq(task);
			rq->wrr.weight_sum += weight - task->wrr.weight;
		}
		task->wrr.weight = weight;
	}
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

