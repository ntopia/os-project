#include "sched.h"

#include <linux/slab.h>

void init_wrr_rq(struct wrr_rq *wrr_rq, struct rq *rq)
{
	raw_spin_lock_init(&wrr_rq->lock);
}
