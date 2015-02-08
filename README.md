dynamic-balancer
================
### Author

* Jared Klingenberger <klinge2@clemson.edu>

A dynamic work balancer using the MPI library. It uses a round-robin approach to
serving workloads. Each workload unit is a number in the interval `[0,4]`
correlated with how long the process should sleep (simulating work).

This program was written for CpSc 3620 (Distributed Programming) at Clemson
University.

How to build
============
Use the Makefile by running `make`.
