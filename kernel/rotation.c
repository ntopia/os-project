#include <linux/kernel.h>
#include <linux/syscalls.h>

#include <uapi/asm-generic/errno-base.h>

int cur_rotation; /* current rotation of device */

/*
 * set rotation function
 */
SYSCALL_DEFINE1(set_rotation, int, degree)
{
	if(degree < 0 || degree >= 360)
		return -EINVAL;
	/*
	 * need a lock here
	 */
	cur_rotation = degree;
	printk("set rotation to %d\n",cur_rotation);
	/*
	 * TODO: do sth here
	 */
	return 0;
}
