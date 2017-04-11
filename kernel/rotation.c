#include <linux/kernel.h>
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
};

/* context of rotation lock */
static LIST_HEAD(acquired);
static LIST_HEAD(pending);
spinlock_t ctx_lock = __SPIN_LOCK_UNLOCKED();

int cur_rotation; /* current rotation of device */


bool check_intersect(int degree1, int range1, int degree2, int range2)
{
	int dist1 = (degree1 - degree2 > 0) ? degree1 - degree2 : 360 + degree1 - degree2;
	int dist2 = 360 - 2 - dist1;
	int dist = (dist1 < dist2) ? dist1 : dist2;
	return dist < range1 + range2;
}


bool find_acquired_lock(int degree, int range, int mode_flag)
{
	struct rotlock_t *rotlock;

	list_for_each_entry(rotlock, &acquired, list) {
		if (check_intersect(degree, range, rotlock->degree, rotlock->range) && (rotlock->mode & mode_flag))
			return true;
	}
	return false;
}


/*
 * set rotation function
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

SYSCALL_DEFINE2(rotlock_read, int, degree, int, range)
{
	struct rotlock_t *new_lock;

	if (degree < 0 || degree >= 360)
		return -EINVAL;

	spin_lock(&ctx_lock);
	printk("** lock read rotation : (%d, %d) **\n", degree, range);

	new_lock = kmalloc(sizeof(struct rotlock_t), GFP_KERNEL);
	new_lock->degree = degree;
	new_lock->range = range;
	new_lock->mode = ROTLOCK_READ;

	if (find_acquired_lock(degree, range, ROTLOCK_READ | ROTLOCK_WRITE)) {
		list_add_tail(&new_lock->list, &pending);
		spin_unlock(&ctx_lock);
		////	TODO: wait
	}
	else {
		list_add_tail(&new_lock->list, &acquired);
	}

	spin_unlock(&ctx_lock);

	return 0;
}

SYSCALL_DEFINE2(rotlock_write, int, degree, int, range)
{
	struct rotlock_t *new_lock;

	if (degree < 0 || degree >= 360)
		return -EINVAL;

	spin_lock(&ctx_lock);
	printk("** lock write rotation : (%d, %d) **\n", degree, range);

	new_lock = kmalloc(sizeof(struct rotlock_t), GFP_KERNEL);
	new_lock->degree = degree;
	new_lock->range = range;
	new_lock->mode = ROTLOCK_WRITE;

	if (find_acquired_lock(degree, range, ROTLOCK_READ | ROTLOCK_WRITE)) {
		list_add_tail(&new_lock->list, &pending);
		spin_unlock(&ctx_lock);
		////	TODO: wait
	}
	else {
		list_add_tail(&new_lock->list, &acquired);
	}

	spin_unlock(&ctx_lock);

	return 0;
}

SYSCALL_DEFINE2(rotunlock_read, int, degree, int, range)
{
	if (degree < 0 || degree >= 360)
		return -EINVAL;

	spin_lock(&ctx_lock);
	printk("** unlock read rotation : (%d, %d) **\n", degree, range);
	spin_unlock(&ctx_lock);

	return 0;
}

SYSCALL_DEFINE2(rotunlock_write, int, degree, int, range)
{
	if (degree < 0 || degree >= 360)
		return -EINVAL;

	spin_lock(&ctx_lock);
	printk("** unlock write rotation : (%d, %d) **\n", degree, range);
	spin_unlock(&ctx_lock);

	return 0;
}

