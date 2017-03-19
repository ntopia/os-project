#include <stdio.h>
#include <stdlib.h>
#include <sys/unistd.h>
#include "test.h"

int main() {
	int nr = 3;
	struct prinfo *prs = calloc(nr, sizeof(struct prinfo));

	int err = syscall(__NR_ptree, prs, &nr);
	if (err < 0) {
		printf("return with error value : %d\n", err);
		return 1;
	}

	return 0;
}
