#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/unistd.h>

#include "test.h"

int main (int argc, char *argv[])
{
	int pid = getpid();
	srand(time(NULL));
	while (1) {
		int x = rand();
		printf("trial[%d]: %d =",pid,x);
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
		puts("");
		usleep(50000);
	}

	return 0;
}
