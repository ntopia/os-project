# project 2 report
team 10 (Han Suhwan, Park Seongwon, Roh Yoonmi)


## Modified files

### kernel
* `arch/arm/include/asm/unistd.h` : changed number of system calls
* `arch/arm/kernel/calls.S` line 392 : added entries in jump table
* `include/linux/rotation.h` : added function to remove locks from finished task
* `include/linux/syscalls.h` line 852 : declared system calls
* `kernel/exit.c` line 721 : remove locks from finished task
* `kernel/rotation.c` : implements system calls


### test program
* `tester/selector.c`: *selector* implementation
* `tester/trial.c`: *trial* implementation
* `tester/test.h`: common header file for test programs


## How to run

### building kernel
```
build
```


### building test program
```
cd tester
make
push selector /root/selector
push trial /root/trial
```


### running test programs (inside artik console)
selector
```
./selector [STARTING_INTEGER]
```

trial
```
./trial [INTEGER_IDENTIFIER]
```


## Lock scheduling policy

It is same as TA policy. (https://github.com/swsnu/osspr2017/issues/58)

Lock can be acquired only when current rotation is inside range of lock.
Writer cannot be overlapped, Readers can overlap each other.
Writer can grap a lock if there are no overlapped locks that are already acquired.
Reader can grap a lock if there are no overlapped writer locks acquired, and there are no (Reader, Writer) pair that those two overlap each other, Reader lock is acquired and Writer lock is pending.

You can see the detail at `check_acquirable()` in `kernel/rotation.c`.

## Implementation

### adding system calls
Actual functions and data structures are implemented in `kernel/rotation.c` and then modified kernel/Makefile to actually build it.

Add system calls as we did in project 1, add entry points in jump table at `arch/arm/kernel/calls.S` and modified number of system calls at `arch/arm/include/asm/unistd.h`.
There was an issue with system call number 384, so we used 385 instead.
Then defined system calls at `include/linux/syscalls.h`.


### defining rotation lock data structure

Defined at `kernel/rotation.c`, this structure are needed to store degree and range of lock, mode of lock(read or write), and task's pid.

Since we are managing it with kernel list, it needs to have list_head variable.

Finally, it has a mutex to implement wait condition.


### defining rotation lock context

We need two lists: one manages acquired locks and the other manages pending locks. These two lists are defined at line 24~25 in `kernel/rotation.c`.

Finally, we need a spinlock object that synchronizes accesses to those lists.
It is defined at line 26 in `kernel/rotation.c`.


### implementing wait condition

To implement waiting, each rotation lock has a mutex.
When rotation lock requests cannot be acquired immediately, it initializes mutex in that rotation lock, and calls mutex lock twice.
Then the second lock call cannot be granted thus in deadlock condition.
When pending rotation lock becomes possible to acquire a lock, it unlocks its mutex variable, then the second mutex lock call now can be granted, and deadlock is resolved.

The tricky thing is that if you need to receive interrupts when waiting for mutex, you should use `mutex_lock_interruptible()` or `mutex_lock_killable()` instead of `mutex_lock()`.


### implementing helper functions

#### rot_distance

Returns minimum distance between two points.

#### check_overlap

Given two (degree, range) pair, check whether those two ranges are overlapped.

#### check_contains

Given a range and a point, check if given point is inside of range.

#### find\_overlapped\_acquired\_lock

Given a range and mode, returns first lock that is acquired, matches given mode, and is overlapped.
If no locks are found, returns NULL.

#### find\_acquired\_lock

Given mode, pid and range, find a lock that matches all of given condition.
If no locks are found, returns NULL.

#### check_acquirable

Given lock, checks whether given lock can be acquired, following our policy.

#### resolve_pending

Called when locks are unlocked or rotation is changed.
Traverses pending list and acquire valid locks.

Returns total number of awoken processes.

#### remove\_all\_rotlocks\_for\_task

Unlocks all locks which belongs to given task's pid.


### implementing system calls

#### set_rotation

First check whether input is valid, then wait until it can grap a spin lock.

When spin lock is granted, change rotation value and unlock.

It returns resolve_pending, which will return number of newly acquired rotation locks.

#### rotlock\_read and rotlock\_write

First check input value, then create a new lock and wait for spin lock.

After spin lock is granted, check if it can be acquired immediately.
If it is, add it to acquired list and returns.
If not, add it to pending list and calls mutex_lock twice, then this system call will be finished when deadlock is resolved.

#### rotunlock\_read and rotunlock\_write

First check input value, then find acquired lock that matched given condition.
If there is one, release it.

Finally, resolve pending rotation locks and free allocated memory.

