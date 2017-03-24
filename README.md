# project 0 report
team 10 (Han Suhwan, Park Seongwon, Roh yoonmi)

## Abstraction

This project was to add ptree system call in artik kernel


## Implementation

### adding prinfo structure

added in include/linux/prinfo.h


### adding system call

Defined actual function sys\_ptree in kernel/ptree.c
since this file did not exist in original kernel source, we had to modify kernel/Makefile to actually compile this file.

Then we added it in system call table at arch/arm/kernel/calls.S and changed \_\_NR\_syscalls from 380 to 384 at arch/arm/include/asm/unistd.h, because kernel checks this value with actual number of system calls and if they are not equal, it returns an error.
number of system calls are aligned by 4, so value had to be 384, not 381.

Then we added this function at include/linux/syscalls.h to let assembly code run this function.


### implementing actual function

First, check if input values are valid. then allocate buffer in kernel space and call do\_ptree function.

do\_ptree performs DFS and adds task informations at buffer in pre-order. and returns number of tasks.

Finally, sys\_ptree copies values from do\_ptree to user space and returns.


### implementing test program
TODO
