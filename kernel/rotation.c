#include <linux/kernel.h>
#include <linux/syscalls.h>

#include <uapi/asm-generic/errno-base.h>

enum rotlock_mode {
	ROTLOCK_READ = 0,
	ROTLOCK_WRITE = 1
};

struct rotlock_t {
	int degree, range;
	enum rotlock_mode mode;
	pid_t pid;
};

////	Contexts
struct list_head acquired;
struct list_head pending;
spinlock_t ctx_lock = __SPIN_LOCK_UNLOCKED();

int cur_rotation; /* current rotation of device */

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
	if (degree < 0 || degree >= 360)
		return -EINVAL;

	spin_lock(&ctx_lock);
	printk("** lock read rotation : (%d, %d) **\n", degree, range);
	spin_unlock(&ctx_lock);

	return 0;
}

SYSCALL_DEFINE2(rotlock_write, int, degree, int, range)
{
	if (degree < 0 || degree >= 360)
		return -EINVAL;

	spin_lock(&ctx_lock);
	printk("** lock write rotation : (%d, %d) **\n", degree, range);
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

