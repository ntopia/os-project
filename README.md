# Project 3 report
team 10 (Han Suhwan, Park Seongwon, Roh Yoonmi)

## Modified files

### kernel
* arch/arm/include/asm.unistd.h
* arch/arm/kernel/calls.S
* include/linux/init_task.h
* include/linux/interrupt.h
* include/linux/sched.h
* include/linux/sched/wrr.h
* include/linux/syscalls.h
* include/uapi/linux/sched.h
* kernel/sched/core.c
* kernel/sched/debug.c
* kernel/sched/rt.c
* kernel/sched/sched.h
* kernel/sched/wrr.c
* kernel/softirq.c

### test program
* test/test.h
* test/trial.c

## How to run

### Building kernel
```
build
```

### Building test program
```
cd test
make
push trial /root/trial
```

### Running test program
```
./trial
```

### Implementation

#### Implementing classes
We need to implement three classes, `wrr_sched_class`, `wrr_rq`, and `wrr_entity`.
`wrr_sched_class` has function pointers to actually run wrr.
`wrr_rq` is used in `rq`, and have a lock and list of wrr tasks. It also has number of tasks in `rq` and sum of their weight to use in loadbalancing.
`wrr_entity` is used in `task_struct` and has variables used in wrr such as weight.

#### Initializing data
On initializing scheduler, call loadbalance to move existing tasks to newly created scheduler.
When initializing `rq`, also initialize `wrr_rq`, initializing lock, list and variables.
On enqueueing wrr task, initialize wrr entity with default weight and timeslice if they are not already set.

#### Implementing system calls
In setweight, first check whether input is correct, and if user is root or owner of that task.
Root can set weight to any valid value, owner can only decrease weight of task.
Getweight first check if input pid exists and that task's policy is wrr. if it is, return weight of that task.

#### Changes in existing classes
Scheduler classes have priority, and in spec, priority of wrr is between rt and fair. so set next class of rt to wrr, and next class of wrr to fair.
//TODO: interrupt..?

#### Implementing WRR class functions
//TODO

#### Implementing loadbalance
When triggered, first find runqueue with minimum weight sum and maximum weight sum by traversing online cpus. We used `rcu_read_lock` to assure synchronization.
If it is possible to migrate, then use `double_rq_lock` to hold locks for both rqs, and traverse tasks in rq with maximum weight sum and find task with maximum migratable task.
If we found one, then dequeue that task from maximum rq and enqueue it in minimum rq, then unlock both locks.

#### Setting WRR to default scheduler
Change codes in `include/linux/init_task.h`, `kernel/kthread.c`, and `kernel/sched/core.c`. Change their policy to `SCHED_WRR`.

#### Debugging
//TODO

### Investigation
//TODO

### Optimizations
//TODO
