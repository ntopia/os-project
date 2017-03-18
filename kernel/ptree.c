/*
 *	linux/kernel/ptree.c
 *
 *	snu os project 1
 *	team 10
 */

#include <linux/kernel.h>
#include <linux/syscalls.h>

#include <linux/prinfo.h>

#include <uapi/asm-generic/errno-base.h>

/*
 * entry point for ptree
 */
SYSCALL_DEFINE2(ptree, struct prinfo *, buf, int *, nr)
{
	if(buf == NULL || nr == NULL || buf->pid == 0)
	{
		return -EINVAL;
	}

	/*
	 * TODO: add do_ptree function
	 */
	return 0;
}
