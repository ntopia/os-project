# project 1 report
team 10 (Han Suhwan, Park Seongwon, Roh Yoonmi)

## Abstraction

This project was to add ptree system call in artik kernel


## What we modified

### kernel

* `include/linux/prinfo.h` : declaration of `struct prinfo`
* `include/linux/syscalls.h` line 853 : declaration of `sys_ptree`
* `arch/arm/kernel/calls.S` line 392 : entry in jump table
* `arch/arm/include/asm/unistd.h` line 18 : number of syscalls
* `kernel/ptree.c` : implementation of `sys_ptree`

### test program

* `tester/proj1/test.c` : implementation of test program
* `tester/proj1/Makefile` : makefile of test program


## How to Build

### kernel

```
build
```

### test program

```
cd tester/proj1
make
```


## Implementation

### adding prinfo structure

added in `include/linux/prinfo.h`


### adding system call

Defined actual function `sys_ptree` in `kernel/ptree.c`
since this file did not exist in original kernel source, we had to modify kernel/Makefile to actually compile this file.

Then we added it in system call table at `arch/arm/kernel/calls.S` and changed `__NR_syscalls` from 380 to 384 at `arch/arm/include/asm/unistd.h`, because kernel checks this value with actual number of system calls and if they are not equal, it returns an error.
number of system calls are aligned by 4, so value had to be 384, not 381.

Then we added this function at `include/linux/syscalls.h` to let assembly code run this function.


### implementing actual function

#### abstraction
First, check if input values are valid in `sys_ptree`.
Then Allocate buffer in kernel space and call `do_ptree` function.
`do_ptree` performs DFS and adds task informations at buffer in pre-order.
And it returns number of tasks.
Finally, `sys_ptree` copies values from `do_ptree` to user space, check validity, and returns number of tasks.


#### checking validity
`sys_ptree` returns error codes below.
* return `-EINVAL`: input value is invalid
    * `buf` is NULL
    * `nr` is NULL
    * value of `nr` is equal or less then 1
    * failed to get value of `nr`
* return `-ENOMEM`: failed to allocate kernel buffer
* return `-EFAULT`: failed to copy results to user memory
    * failed to copy `prinfo` struct list to user memory `buf`
    * failed to copy number of processes to user memory `nr`

#### implementing DFS
Depth-First-Search the tasks with non-recursive function `do_ptree`.
Starting with `init_task`, it gets the next task with `get_next_struct`.

`get_next_struct` gets the current `task_struct`
and returns next struct ordered by DFS-preorder.
If current task has child, the next task is its child.
Unless it has child, it returns the next sibling.
If it has no next sibling, it finds its parent's sibling, recursively.
It returns NULL when it is the last one in DFS-preorder.

For the code readibility and reusebility,
getting first child and next sibling with `get_first_child` and `get_next_sibling`.
They check if it has one or returns NULL.

### implementing test program
TODO
