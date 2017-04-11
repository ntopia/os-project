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
};


int cur_rotation; /* current rotation of device */

/*
 * set rotation function
 */
SYSCALL_DEFINE1(set_rotation, int, degree)
{
	if (degree < 0 || degree >= 360)
		return -EINVAL;
	/*
	 * need a lock here
	 */
	cur_rotation = degree;
	printk("set rotation to %d\n", cur_rotation);
	/*
	 * TODO: do sth here
	 */
	return 0;
}

SYSCALL_DEFINE2(rotlock_read, int, degree, int, range)
{
	if (degree < 0 || degree >= 360)
		return -EINVAL;
	/*
	 * need a lock here
	 */
	printk("** lock read rotation : (%d, %d) **\n", degree, range);
	/*
	 * TODO: do sth here
	 */
	return 0;
}

SYSCALL_DEFINE2(rotlock_write, int, degree, int, range)
{
	if (degree < 0 || degree >= 360)
		return -EINVAL;
	/*
	 * need a lock here
	 */
	printk("** lock write rotation : (%d, %d) **\n", degree, range);
	/*
	 * TODO: do sth here
	 */
	return 0;
}

SYSCALL_DEFINE2(rotunlock_read, int, degree, int, range)
{
	if (degree < 0 || degree >= 360)
		return -EINVAL;
	/*
	 * need a lock here
	 */
	printk("** unlock read rotation : (%d, %d) **\n", degree, range);
	/*
	 * TODO: do sth here
	 */
	return 0;
}

SYSCALL_DEFINE2(rotunlock_write, int, degree, int, range)
{
	if (degree < 0 || degree >= 360)
		return -EINVAL;
	/*
	 * need a lock here
	 */
	printk("** unlock write rotation : (%d, %d) **\n", degree, range);
	/*
	 * TODO: do sth here
	 */
	return 0;
}

