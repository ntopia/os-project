#include <linux/kernel.h>
#include <linux/syscalls.h>

#include <uapi/asm-generic/errno-base.h>

/*
 * set rotation function
 */
SYSCALL_DEFINE1(set_rotation, int, degree)
{
	/*
	 * TODO: do sth here
	 */
	 return 0;
}
