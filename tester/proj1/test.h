#ifndef _PROJ1_TEST_H
#define _PROJ1_TEST_H


#include <sys/types.h>

#define __NR_ptree 380

struct prinfo {
	long state;
	pid_t pid;
	pid_t parent_pid;
	pid_t first_child_pid;
	pid_t next_sibling_pid;
	long uid;
	char comm[64];
};

#endif
