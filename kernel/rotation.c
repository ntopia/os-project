#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/syscalls.h>

#include <uapi/asm-generic/errno-base.h>

enum rotlock_mode {
	ROTLOCK_READ = 1,
	ROTLOCK_WRITE = 2
};

struct rotlock_t {
	int degree, range;
	enum rotlock_mode mode;
	pid_t pid;
	struct list_head list;
	struct mutex lock;
};

/* context of rotation lock */
static LIST_HEAD(acquired);
static LIST_HEAD(pending);
DEFINE_SPINLOCK(ctx_lock);

int cur_rotation; /* current rotation of device */


/*
 * return distance of two rotation
 */
int rot_distance(int rot1, int rot2)
{
	int dist1 = (rot1 - rot2 > 0) ? rot1 - rot2 : 360 + rot1 - rot2;
	int dist2 = 360 - dist1;
	return (dist1 < dist2) ? dist1 : dist2;
}

/*
 * return true if two ranges are overlapped
 */
bool check_overlap(int degree1, int range1, int degree2, int range2)
{
	return rot_distance(degree1, degree2) <= range1 + range2;
}

/*
 * return true if given range contains rot
 */
bool check_contains(int degree, int range, int rot)
{
	return rot_distance(degree, rot) <= range;
}

/*
 * find overlapped & acquired lock with [degree-range, degree+range]
 * it filters result by mode_flag
 */
struct rotlock_t *find_overlapped_acquired_lock(int degree, int range, int mode_flag)
{
	struct rotlock_t *rotlock;

	list_for_each_entry(rotlock, &acquired, list) {
		if ((rotlock->mode & mode_flag)
			&& check_overlap(degree, range, rotlock->degree, rotlock->range))
			return rotlock;
	}
	return NULL;
}

/*
 * find lock by pid
 */
struct rotlock_t *find_lock(pid_t pid)
{
	struct rotlock_t *rotlock;

	list_for_each_entry(rotlock, &acquired, list) {
		if (rotlock->pid == pid)
			return rotlock;
	}
	list_for_each_entry(rotlock, &pending, list) {
		if (rotlock->pid == pid)
			return rotlock;
	}
	return NULL;
}

/*
 * traverse pending list and acquire valid locks
 * TODO: fix it to follow our policy
 */
void resolve_pending(void)
{
	struct rotlock_t *rotlock, *next_rotlock;

	spin_lock(&ctx_lock);
	list_for_each_entry_safe(rotlock, next_rotlock, &pending, list) {
		int degree = rotlock->degree;
		int range = rotlock->range;
		if (check_contains(degree,range, cur_rotation)
				&& !find_overlapped_acquired_lock(degree, range, ROTLOCK_READ | ROTLOCK_WRITE)) {
			mutex_unlock(&rotlock->lock);
			list_del(&rotlock->list);
			list_add_tail(&rotlock->list, &acquired);
		}
	}
	spin_unlock(&ctx_lock);
}

/*
 * set_rotation syscall
 */
SYSCALL_DEFINE1(set_rotation, int, degree)

{
	if (degree < 0 || degree >= 360)
		return -EINVAL;

	spin_lock(&ctx_lock);
	cur_rotation = degree;
	printk("set rotation to %d\n", cur_rotation);
	spin_unlock(&ctx_lock);

	return 0;
}


/*
 * rotlock_read syscall
 */
SYSCALL_DEFINE2(rotlock_read, int, degree, int, range)
{
	struct rotlock_t *new_lock;

	if (degree < 0 || degree >= 360)
		return -EINVAL;

	new_lock = kmalloc(sizeof(struct rotlock_t), GFP_KERNEL);
	new_lock->degree = degree;
	new_lock->range = range;
	new_lock->mode = ROTLOCK_READ;
	new_lock->pid = task_pid_nr(current);

	spin_lock(&ctx_lock);
	printk("** lock read rotation : (%d, %d) **\n", degree, range);

	if (!check_contains(degree, range, cur_rotation)
			|| find_overlapped_acquired_lock(degree, range, ROTLOCK_READ | ROTLOCK_WRITE)) {

		printk("there is an acquired lock overlapped with me.\n");
		printk("or cur_rotation is not mine.\n");
		printk("we should wait.\n");

		list_add_tail(&new_lock->list, &pending);
		spin_unlock(&ctx_lock);
		/*
		 * wait until acquired
		 */
		mutex_init(&new_lock->lock);
		mutex_lock(&new_lock->lock);
		mutex_lock(&new_lock->lock);
		mutex_unlock(&new_lock->lock);
		spin_lock(&ctx_lock);
	} else {
		printk("ok. immediately get a lock!\n");
		list_add_tail(&new_lock->list, &acquired);
	}

	spin_unlock(&ctx_lock);

	return 0;
}


/*
 * rotlock_write syscall
 */
SYSCALL_DEFINE2(rotlock_write, int, degree, int, range)
{
	struct rotlock_t *new_lock;

	if (degree < 0 || degree >= 360)
		return -EINVAL;

	new_lock = kmalloc(sizeof(struct rotlock_t), GFP_KERNEL);
	new_lock->degree = degree;
	new_lock->range = range;
	new_lock->mode = ROTLOCK_WRITE;
	new_lock->pid = task_pid_nr(current);

	spin_lock(&ctx_lock);
	printk("** lock write rotation : (%d, %d) **\n", degree, range);

	if (!check_contains(degree, range, cur_rotation)
			|| find_overlapped_acquired_lock(degree, range, ROTLOCK_READ | ROTLOCK_WRITE)) {

		printk("there is an acquired lock overlapped with me.\n");
		printk("or cur_rotation is not mine.\n");
		printk("we should wait.\n");

		list_add_tail(&new_lock->list, &pending);
		spin_unlock(&ctx_lock);
		/*
		 * wait until acquired
		 */
		mutex_init(&new_lock->lock);
		mutex_lock(&new_lock->lock);
		mutex_lock(&new_lock->lock);
		mutex_unlock(&new_lock->lock);
		spin_lock(&ctx_lock);
	} else {
		printk("ok. immediately get a lock!\n");
		list_add_tail(&new_lock->list, &acquired);
	}

	spin_unlock(&ctx_lock);

	return 0;
}


/*
 * rotunlock_read syscall
 */
SYSCALL_DEFINE2(rotunlock_read, int, degree, int, range)
{
	pid_t pid;
	struct rotlock_t *lock;

	if (degree < 0 || degree >= 360)
		return -EINVAL;

	pid = task_pid_nr(current);
	spin_lock(&ctx_lock);
	printk("** unlock read rotation : (%d, %d) **\n", degree, range);

	lock = find_lock(pid);
	if (lock)
		list_del(&lock->list);
	else
		printk("couldnt find lock! something wrong!\n");

	spin_unlock(&ctx_lock);
	resolve_pending();

	kfree(lock);
	return 0;
}


/*
 * rotunlock_write syscall
 */
SYSCALL_DEFINE2(rotunlock_write, int, degree, int, range)
{
	pid_t pid;
	struct rotlock_t *lock;

	if (degree < 0 || degree >= 360)
		return -EINVAL;

	pid = task_pid_nr(current);
	spin_lock(&ctx_lock);
	printk("** unlock write rotation : (%d, %d) **\n", degree, range);

	lock = find_lock(pid);
	if (lock)
		list_del(&lock->list);
	else
		printk("couldnt find lock! something wrong!\n");

	spin_unlock(&ctx_lock);
	resolve_pending();

	kfree(lock);
	return 0;
}

