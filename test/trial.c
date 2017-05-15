#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/unistd.h>
#include <linux/sched.h>
#include <sched.h>

#include "test.h"

clock_t time_weight[20];
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
 
		for (int cnt = 0; cnt < TOT_CNT; cnt++) {
			clock_t t = clock();
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
			t = clock() - t;
			printf("\t[%d] %lf\n", cnt, ((double)t)/CLOCKS_PER_SEC);
			time_weight[weight] += t;
		}
		printf("----------\n");
		printf("sum: %lf\tavg: %lf\n\n", ((double)time_weight[weight])/CLOCKS_PER_SEC, ((double)time_weight[weight])/CLOCKS_PER_SEC/TOT_CNT);
	}

	FILE *f = fopen("out.txt", "w");
	for (int i = 0; i < 20; i++) {
		fprintf(f, "weight: %d\tsum: %lf\tavg: %lf\n", i+1, ((double)time_weight[i])/CLOCKS_PER_SEC, ((double)time_weight[i])/CLOCKS_PER_SEC/TOT_CNT);

	}	
	fclose(f);
	return 0;
}
