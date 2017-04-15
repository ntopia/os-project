# project 2 report
team 10 (Han Suhwan, Park Seongwon, Roh Yoonmi)


## Modified files

### kernel
* `arch/arm/include/asm/unistd.h`: changed number of system calls
* `arch/arm/kernel/calls.S`: added entries in jump table
* `include/linux/rotation.h`: added function to remove locks from finished task
* `include/linux/syscalls.h`: declared system calls
* `kernel/exit.c`: remove locks from finished task
* `kernel/rotation.c`: implementations


### tester
* `tester/rot_test.c`: 
* `tester/test.c`: 
* `tester/test.h`: 


## How to run

### building kernel
```
build
```


### building test program
TODO


### running test programs
TODO


## Lock scheduling policy

Lock can be acquired only when current rotation is inside range of lock.
Writer cannot be overlapped, Readers can overlap each other.
Writer can grap a lock if there are no overlapped locks that are already acquired.
Reader can grap a lock if there are no overlapped writer locks acquired, and there are no (Reader, Writer) pair that those two overlap each other, Reader lock is acquired and Writer lock is pending.


## Implementation

### adding system calls
Actual functions and data structures are implemented in `kernel/rotation.c` and then modified kernel/Makefile to actually build it.

Add system calls as we did in project 1, add entry points in jump table at `arch/arm/kernel/calls.S` and modified number of system calls at `arch/arm/include/asm/unistd.h`. there was an issue with system call number 384, so we used 385 instead. then defined system calls at `include/linux/syscalls.h`.


### defining rotation lock data structure

Defined at `kernel/rotation.c`, this structure are needed to store degree and range of lock, mode of lock(read or write), and pid.

Since we are managing it with kernel list, it needs to have list_head variable.

Finally, it has a mutex to implement wait condition.


### implementing wait condition

To implement waiting, each rotation lock has a mutex.
When rotation lock requests cannot be acquired immediately, it initializes mutex in that rotation lock, and calls mutex lock twice. then the second lock call cannot be granted thus in deadlock condition.
When pending rotation lock becomes possible to acquire a lock, it unlocks its mutex variable, then the second mutex lock call now can be granted, and deadlock is resolved.


### implementing helper functions

#### rot_distance

Returns minimum distance between two points.

#### check_overlap

Given two (degree, range) pair, check whether those two ranges are overlapped.

#### check_contains

Given a range and a point, check if given point is inside of range.

#### find\_overlapped\_acquired\_lock

Given a range and mode, returns first lock that is acquired , matches given mode, and is overlapped. if no locks are found, returns NULL.

#### find\_acquired\_lock

Given mode, pid and range, find a lock that matches all of given condition. if no locks are found, returns NULL.

#### check_acquirable

Given lock, checks whether given lock can be acquired, following our policy.

#### resolve_pending

Called when locks are unlocked or rotation is changed. traverses pending list and acquire valid locks.

Returns total number of awoken processes.

#### remove\_all\_rotlocks\_for\_task

Unlocks all locks which belongs to given process id.

### implementing system calls

#### set_rotation

First check whether input is valid, then wait until it can grap a spin lock.

When spin lock is granted, change rotation value and unlock.

It returns resolve_pending, which will return number of newly acquired rotation locks.

#### rotlock\_read and rotlock\_write

First check input value, then create a new lock and wait for spin lock.

After spin lock is granted, check if it can be acquired immediately. if it is, add it to acquired list and returns. if not, add it to pending list and calls mutex_lock twice, then this system call will be finished when deadlock is resolved.


#### rotunlock\_read and rotunlock\_write

First check input value, then find acquired lock that matched given condition. if there is one, release it.

Finally, resolve pending rotation locks and free allocated memory.
