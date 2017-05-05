#include <stdio.h>
#include <sys/unistd.h>
#include "test.h"

int main()
{
	printf("call sched_setweight\n");
	syscall(__NR_sched_setweight, NULL, 0);
	
	printf("call sched_getweight\n");
	syscall(__NR_sched_getweight, NULL);

	return 0;
}
