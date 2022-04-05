# Benchmarking system calls
This code measures the CPU cycles for various system calls.

## Compile and Execute
```
$ make
$ taskset -c 0 ./syscall_benchmark
```

## Results
Plot the results using the MATLAB code "syscall_benchmark.m"

<img src="Results/syscall_benchmark.svg" width="750px" height="auto">

## Experimental Setup
We have included the results in the "Results" folder for the following setup:

**Intel® Core™ i5-6440HQ CPU @ 2.60GHz × 4**

**Ubuntu 20.04.4 LTS**
