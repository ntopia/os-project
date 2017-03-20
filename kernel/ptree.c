/*
 *	linux/kernel/ptree.c
 *
 *	snu os project 1
 *	team 10
 */

#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/slab.h>

#include <linux/prinfo.h>

#include <uapi/asm-generic/errno-base.h>

long do_ptree(struct prinfo *kbuf, int *nr_value){
	/*
	 * push prinfo struct to kbuf in Depth First Search order.
	 * modify nr_value, 
	 * if total number of entries is smaller than that.
	 * 
	 * return value: total number of entries on success, or the error code
	 * 	-EFAULT: if buf are outside the accessible address space
	 */
	long entry_num = 0;
	read_lock(&tasklist_lock);
		
	read_unlock(&tasklist_lock);
	

	if (*nr_value > entry_num) *nr_value = entry_num;
	return entry_num;
}


/*
 * entry point for ptree
 */

SYSCALL_DEFINE2(ptree, struct prinfo __user *, buf, int __user *, nr)
{
	/*
	 * prinfo: process information struct defined in include/linux/prinfo.h
	 * buf: buffer for the process data
	 * nr: size of this buffer
	 * return value: total number of entries on success, or the error code
	 * 	-EINVAL: if buf or nr are null,
	 * 	or if the number of entries is less than 1
	 * 	-EFAULT: if buf or nr are outside the accessible address space
	 */
	int nr_value;
	struct prinfo *kbuf;
	long result;

	if (buf == NULL || nr == NULL)
		return -EINVAL;
	if (get_user(nr_value, nr))
		return -EFAULT;
	if (nr_value <= 1)
		return -EINVAL;
	
	kbuf = (struct prinfo*)kmalloc(nr_value * sizeof(struct prinfo), GFP_KERNEL);
	if (kbuf == NULL)
		return -ENOMEM;
	result = do_ptree(kbuf, &nr_value);

	if (copy_to_user(buf, kbuf, nr_value*sizeof(struct prinfo)))
		return -EFAULT;
	if (put_user(nr_value, nr))
		return -EFAULT;
	return result;
}
