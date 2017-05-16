#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/unistd.h>
#include <linux/sched.h>
#include <sched.h>
#include <sys/time.h>

#include "test.h"

double time_weight[20];
int TOT_CNT = 2;

int main (int argc, char *argv[])
{
	int x = 1000000007;
	srand(time(NULL));
	int pid = getpid();
	printf("trial: pid[%d]\n",pid);

	for (int weight = 0; weight < 20; weight++) {
		time_weight[weight] = 0;
		printf("==== weight <%d> ====\n", weight+1);
		syscall(__NR_sched_setweight, pid, weight+1);

		struct timeval tv1, tv2;
		for (int cnt = 0; cnt < TOT_CNT; cnt++) {
			gettimeofday(&tv1, NULL);
			if (argc > 1)
				x = rand();
			else
				x = 1000000007;

			char first = 1;
			while ((x&1) == 0) {
				if (first) {
					first = 0;
					printf(" 2");
				}
				else
					printf(" * 2");
				x /= 2;
			}
			for(int d=3;x>1;d+=2) {
				while (x%d == 0) {
					if (first) {
						first = 0;
						printf(" %d", d);
					}
					else
						printf(" * %d", d);
					x /= d;
				}
			}
			gettimeofday(&tv2, NULL);

			double elapsed = (double)(tv2.tv_usec - tv1.tv_usec) / 1000000 + (double)(tv2.tv_sec - tv1.tv_sec);
			printf("\t[%d] %lf\n", cnt, elapsed);
			time_weight[weight] += elapsed;
		}
		printf("----------\n");
		printf("sum: %lf\tavg: %lf\n\n", time_weight[weight], time_weight[weight]/TOT_CNT);
	}

	FILE *f = fopen("out.txt", "w");
	for (int i = 0; i < 20; i++) {
		fprintf(f, "weight: %d\tsum: %lf\tavg: %lf\n", i+1, time_weight[i], time_weight[i]/TOT_CNT);
	}	
	fclose(f);
	return 0;
}
