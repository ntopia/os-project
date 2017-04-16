#include <stdio.h>
#include <stdbool.h>
#include <sys/unistd.h>

#include "test.h"

char atoi(char *s, int *ret)
{
	*ret = 0;
	for(int i=0;s[i] && s[i] != ' ' && s[i] != '\n';i++) {
		if(s[i]<'0' || s[i]>'9') {
			puts("identifier must be an integer");
			return -1;
		}
		*ret *= 10;
		*ret += s[i]-'0';
	}
	return 0;
}

char buf[12];

int main (int argc, char *argv[])
{
	if (argc < 2 || argc > 2) {
		puts("usage: selector [identifier]");
		return 0;
	}
	int id;
	if (atoi(argv[1], &id))
		return 0;
	while (1) {
		int ret = syscall(__NR_rotlock_read, 90, 90);
		if (ret == -1) {
			puts("cannot acquire lock");
			return 0;
		}
		FILE * file = fopen("integer","r");
		if (file == NULL) {
			puts("cannot find file \'integer\'");
			return 0;
		}
		ret = fread(buf, sizeof(char), sizeof(buf), file);
		int x;
		if (atoi(buf, &x))
			return 0;
		printf("trial-%d: %d =",id,x);
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
		fclose(file);
		ret = syscall(__NR_rotunlock_read, 90, 90);
		if (ret == -1) {
			puts("cannot release lock");
			return 0;
		}
	}

	return 0;
}
