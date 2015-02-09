dynamic-balancer
================
### Author

* Jared Klingenberger <klinge2@clemson.edu>

A static and a dynamic work balancer using the MPI library. Each workload unit
is a number in the interval `[0,4]` correlated with how long the process should
sleep (simulating work).

This program was written for CpSc 3620 (Distributed Programming) at Clemson
University.

Discussion
==========

### Static workload balancer

The [static balancer](static_mpi.c) uses a contiguous block
distribution algorithm. The main workload (an array found on the master node) is
divided up into `n` blocks, where n is the number of MPI processes. Rather than 
using the naïve approach of giving each process `WORKLOAD_SIZE/n` units, the 
algorithm calculates the mean and standard deviation of the total workload and 
makes sure each process gets roughly mean +/- 5% (z score 0.130). This makes the
workload fairly evenly distributed among processes.

### Dynamic workload balancer

The [dynamic balancer](dynamic_mpi.c) uses a round-robin approach to serving workloads. A single
unit is given to each worker node at a time. When the work is complete, the
node returns the finished workload to the master node with additional elapsed
time data. If there is more work to be done, the master node will send the
worker a new workload. This continues until all workloads are complete.

Known issues
============

* The static balancing algorithm could use some love and care. I am not sure the
z-score is reasonable, or if it should be scaled according to the number of
workers.
* Currently, the master node also acts as a worker. The dynamic balancer could
be optimized for increasing `n` if the master node dropped all worker
responsibilities and only listened for workload returns. This would increase the
throughput of all nodes.

How to build
============
Use the Makefile by running `make`.

Pretty graphs
=============
![static](https://cloud.githubusercontent.com/assets/941126/6102244/a6dc11f2-b000-11e4-8931-952ef42b1be5.png)

![dynamic](https://cloud.githubusercontent.com/assets/941126/6102243/a6dbaae6-b000-11e4-9c0f-520b9b0e4875.png)
