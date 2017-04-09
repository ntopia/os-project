#include <stdio.h>
#include <errno.h>
#include <sys/unistd.h>
#include "test.h"

int main()
{
	printf("call rotlock_read\n");
	syscall(__NR_rotlock_read, 180, 10);
	printf("call rotlock_write\n");
	syscall(__NR_rotlock_write, 180, 10);
	printf("call rotunlock_read\n");
	syscall(__NR_rotunlock_read, 180, 10);
	printf("call rotunlock_write\n");
	syscall(__NR_rotunlock_write, 180, 10);

	return 0;
}
