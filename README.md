# Project 1: System Inspector

Writer: Ankita Khatri 
University of San Francisco  
CS 326: Operating Systems  

## About This Project
This project is a C program that grabs information on your operating system from the proc file directory. You can access system information, hardware information, task information, and a see a live view of your load average, cpu usage, and memory usage.

### What is proc?
The proc directory is the process information virtual file system. There are virtual files inside proc including all the task directories, and information on the cpu, memory, kernel version, etc. 

### Program Options
Each portion of the display can be toggled with command line options. Here are the options:
```bash
$ ./inspector -h
Usage: ./inspector [-ahlrst] [-p procfs_dir]

Options:
    * -a              Display all (equivalent to -lrst, default)
    * -h              Help/usage information
    * -l              Task List
    * -p procfs_dir   Change the expected procfs mount point (default: /proc)
    * -r              Hardware Information
    * -s              System Information
    * -t              Task Information
```
The task list, hardware information, system information, and task information can all be turned on/off with the command line options. By default, all of them are displayed.

### Included Files
There are several files included. These are:
   - <b>Makefile</b>: Including to compile and run the program.
   - <b>inceptor.c</b>: The file that contains all the code to display System Information, Hardware Information, Task Information, or Live View, depending on the flags you choose.


To compile and run:

```bash
make
./inspector
```


### Program Output
```bash
inspector.c:921:main(): View options selected: hardware system task_list
System Information
------------------
Hostname: akhatri-vm 
Kernel Version: 5.2.9-arch1-1-ARCH 
Uptime: 22 days, 2 hours, 42 minutes, 32 seconds
Hardware Information
--------------------
CPU Model: AMD EPYC Processor (with IBPB) 
Processing Units: 2
Load Average (1/5/15 min): 0.000.000.00
CPU Usage:    [--------------------] 0.5%
Memory Usage: [###-----------------] 17.6% (0.2 GB / 1.0 GB)
Task Information
----------------
Tasks Running: 109

  PID |        State |                 Task Name |            User | Tasks
------+--------------+---------------------------+-----------------+-------
14437 |      running |                 inspector |         akhatri |     1 
14437 |      running |                 inspector |         akhatri |     1 
    1 |     sleeping |                   systemd |            root |     1 
    2 |     sleeping |                  kthreadd |            root |     1 
    3 |         idle |                    rcu_gp |            root |     1 
    4 |         idle |                rcu_par_gp |            root |     1 
    6 |         idle |                 kworker/0 |            root |     1 
    8 |         idle |              mm_percpu_wq |            root |     1 
```

## Testing

To execute the test cases, use `make test`. To pull in updated test cases, run `make testupdate`. You can also run a specific test case instead of all of them:

```
# Run all test cases:
make test

# Run a specific test case:
make test run=4

# Run a few specific test cases (4, 8, and 12 in this case):
make test run='4 8 12'
```