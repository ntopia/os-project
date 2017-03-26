#include <stdio.h>
#include <stdlib.h>
#include <sys/unistd.h>
#include "test.h"

void print_prinfo(struct prinfo *pr, int indent) {
	for (int i = 0; i < indent; ++i) {
		printf("  ");
	}
	printf("%s,%d,%ld,%d,%d,%d,%ld\n",
		pr->comm, pr->pid, pr->state,
		pr->parent_pid, pr->first_child_pid, pr->next_sibling_pid, pr->uid);
}

int main() {
	int nr = 3;
	struct prinfo *prs = calloc(nr, sizeof(struct prinfo));

	int err = syscall(__NR_ptree, prs, &nr);
	if (err < 0) {
		free(prs);
		printf("return with error value : %d\n", err);
		return 1;
	}

	pid_t *pid_stack = calloc(nr, sizeof(pid_t));
	int top = 0;

	int indent = 0;
	for (int i = 0; i < nr; ++i) {
		while (top > 0 && pid_stack[top - 1] != prs[i].parent_pid) {
			--top;
			--indent;
		}

		pid_stack[top++] = prs[i].pid;
		++indent;

		print_prinfo(prs + i, indent);
	}

	free(pid_stack);
	free(prs);

	return 0;
}
