#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/unistd.h>
#include "test.h"

int main(int argc, char *argv[])
{
	int input;
	long ret;
	FILE* fp;
	if (argc != 2)
		return -EINVAL;
	input = atoi(argv[1]);
	while (1) {
		ret = syscall(__NR_rotlock_write, 90, 90);
		if (ret == -EINVAL || ret == -EINTR)
			break;

		fp = fopen("integer", "w");
		fprintf(fp, "%d\n", input);
		fclose(fp);
		printf("selector: %d\n", input++);
		
		syscall(__NR_rotunlock_write, 90, 90);
	}
	return 0;
}

