#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/unistd.h>
#include <linux/sched.h>
#include <sched.h>
#include "test.h"

#define SCHED_WRR	6

int i, j;
long ret;
struct sched_param param;
char cmd[100];
const char *helps = "\t\t=== commands ===\n"
"  SPW [pid]\t\t: Set [pid] Policy Wrr\n"
"  SW [pid] [new_weight]\t: Set [pid] Weight as [new_weight]\n"
"  GW [pid]\t\t: Get Weight of [pid]\n"
"  H\t\t\t: Help\n"
"  Q (or E)\t\t: Quit\n";

int main()
{
	while (1) {
		printf("\t> input\t: ");
		scanf("%s", cmd);

		for (int i = 0; cmd[i] != '\0'; i++) {
			if (cmd[i] >= 'a') cmd[i] -= 32;
		}

		if (cmd[0] == 'H') {
			printf("%s", helps);
			continue;
		} else if (cmd[0] == 'Q' || cmd[0] == 'E') {
			printf("\t> exit\n");
			break;
		} else {
			ret = 0;
			if (strncmp(cmd, "SPW", 3) == 0) {
				scanf("%d", &i);
				printf("  > Set Policy Wrr - pid [%d]\n", i);
				param.sched_priority = 0;
				ret = sched_setscheduler(i, SCHED_WRR, &param);
				printf("\t  > %s\n", ret?"failed":"successed");
			} else if (strncmp(cmd, "SW", 2) == 0 ) {
				scanf("%d %d", &i, &j);
				printf("  > Set Weight - pid [%d] weight [%d]\n", i, j);
				ret = syscall(__NR_sched_setweight, i, j);
				printf("\t  > %s\n", ret?"failed":"successed");
			} else if (strncmp(cmd, "GW", 2) == 0) {
				scanf("%d", &i);
				printf("  > Get Weight - pid [%d]\n", i);
				ret = syscall(__NR_sched_getweight, i);
				printf("\t  > [%d]'s weight: %ld\n", i, ret);
			} else {
				printf("  > invalid command: [%s]\n", cmd);
				continue;
			}
		}
	}

	return 0;
}
