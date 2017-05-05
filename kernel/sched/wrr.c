#include "sched.h"
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <uapi/asm-generic/errno-base.h>

#include <linux/slab.h>

void init_wrr_rq(struct wrr_rq *wrr_rq, struct rq *rq)
{
	raw_spin_lock_init(&wrr_rq->wrr_lock);
}

/*
 * Set the SCHED_WRR weight of process, as identified by 'pid'.
 * If 'pid' is 0, set the weight for the calling process.
 * System call number 380.
 */
SYSCALL_DEFINE2(sched_setweight, pid_t, pid, int, weight)
{
	/* check task's policy */
	/* check authority */
	/* update the weight */
	printk(KERN_ALERT"set_weight called\n");
	return 0;
}

/*
 * Obtain the SCHED_WRR weight of a process as identified by 'pid'.
 * If 'pid' is 0, return the weight of the calling process.
 * System call number 381.
 */
SYSCALL_DEFINE1(sched_getweight, pid_t, pid)
{
	/* check task's policy */
	/* return the value */
	printk(KERN_ALERT"get_weight called\n");
	return 0;
}

