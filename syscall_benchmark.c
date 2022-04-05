// $ gcc -o syscall_benchmark syscall_benchmark.c -lkeyutils -lnuma

#include <stdio.h>
#include <stdint.h>		// uint64_t
#include <sys/types.h>
#include <time.h>		// time(), clock_gettime()
#include <sys/ioctl.h>
#include <keyutils.h>	// add_key
#include <stdlib.h>
#include <numaif.h>		// get_mempolicy
#include <sys/mman.h>	// pkey_alloc, pkeyfree
#include <unistd.h>

#define rdtscp()({unsigned int result; asm volatile("rdtscp":"=a"(result)::"rdx","rcx","memory"); result;})

#define ROUNDS 1000000

int main(void)
{
	uint32_t t1 = 0;
	uint32_t t2 = 0;
	uint32_t tt = 0;
	uint64_t total = 0;
	uint32_t measurement[ROUNDS] = {0};
	
	// Warmup loop
	for(int i = 0; i < 55555555; i++);
	
	// nothing
	for (int r = 0; r < ROUNDS; r++)
	{
		t1 = rdtscp();
			// Nothing here
		t2 = rdtscp();
		tt = t2-t1;
		measurement[r] = tt;
		total = total + tt;
	}
	FILE *output;
	output = fopen("nothing.txt", "w");
	for(int r = 0; r < ROUNDS; r++)
		fprintf(output, "%u\n", measurement[r]);
	printf("nothing\t\t%lu\n", total / ROUNDS);
	fclose(output);
	
	// getpid
	total = 0;
	for (int r = 0; r < ROUNDS; r++)
	{
		t1 = rdtscp();
			getpid();
		t2 = rdtscp();
		tt = t2-t1;
		measurement[r] = tt;
		total = total + tt;
	}
	output = fopen("getpid.txt", "w");
	for(int r = 0; r < ROUNDS; r++)
		fprintf(output, "%u\n", measurement[r]);
	printf("getpid\t\t%lu\n", total / ROUNDS);
	fclose(output);
	
	// getuid
	total = 0;
	for (int r = 0; r < ROUNDS; r++)
	{
		t1 = rdtscp();
			getuid();
		t2 = rdtscp();
		tt = t2-t1;
		measurement[r] = tt;
		total = total + tt;
	}
	output = fopen("getuid.txt", "w");
	for(int r = 0; r < ROUNDS; r++)
		fprintf(output, "%u\n", measurement[r]);
	printf("getuid\t\t%lu\n", total / ROUNDS);
	fclose(output);
	
	// time
	total = 0;	
	for (int r = 0; r < ROUNDS; r++)
	{
		t1 = rdtscp();
			time(NULL);
		t2 = rdtscp();
		tt = t2-t1;
		measurement[r] = tt;
		total = total + tt;
	}
	output = fopen("time.txt", "w");
	for(int r = 0; r < ROUNDS; r++)
		fprintf(output, "%u\n", measurement[r]);
	printf("time\t\t%lu\n", total / ROUNDS);
	fclose(output);
	
	// read
	total = 0;
	int fd;
	for (int r = 0; r < ROUNDS; r++)
	{
		t1 = rdtscp();
			read(fd, 0, 0);
		t2 = rdtscp();
		tt = t2-t1;
		measurement[r] = tt;
		total = total + tt;
	}
	output = fopen("read.txt", "w");
	for(int r = 0; r < ROUNDS; r++)
		fprintf(output, "%u\n", measurement[r]);
	printf("read\t\t%lu\n", total / ROUNDS);
	fclose(output);
	
	// ioctl
	total = 0;
	for (int r = 0; r < ROUNDS; r++)
	{
		t1 = rdtscp();
			ioctl(fd, 0, 0);
		t2 = rdtscp();
		tt = t2-t1;
		measurement[r] = tt;
		total = total + tt;
	}
	output = fopen("ioctl.txt", "w");
	for(int r = 0; r < ROUNDS; r++)
		fprintf(output, "%u\n", measurement[r]);
	printf("ioctl\t\t%lu\n", total / ROUNDS);
	fclose(output);
	
	// add_key - add a key to the kernel's key management facility
	// $ sudo apt-get install libkeyutils-dev
	// $ gcc -o syscall_benchmark syscall_benchmark.c -lkeyutils
	total = 0;
	for (int r = 0; r < ROUNDS; r++)
	{
		t1 = rdtscp();
			add_key(0, 0, 0, 0, 0);
		t2 = rdtscp();
		tt = t2-t1;
		measurement[r] = tt;
		total = total + tt;
	}
	output = fopen("add_key.txt", "w");
	for(int r = 0; r < ROUNDS; r++)
		fprintf(output, "%u\n", measurement[r]);
	printf("add_key\t\t%lu\n", total / ROUNDS);
	fclose(output);
	
	// get_mempolicy - retrieve NUMA memory policy for a thread
	// $ gcc -o syscall_benchmark syscall_benchmark.c -lnuma
	total = 0;
	for (int r = 0; r < ROUNDS; r++)
	{
		t1 = rdtscp();
			get_mempolicy(0, 0, 0, 0, 0);
		t2 = rdtscp();
		tt = t2-t1;
		measurement[r] = tt;
		total = total + tt;
	}
	output = fopen("get_mempolicy.txt", "w");
	for(int r = 0; r < ROUNDS; r++)
		fprintf(output, "%u\n", measurement[r]);
	printf("get_mempolicy\t%lu\n", total / ROUNDS);
	fclose(output);
	
	// clock_gettime
	struct timespec start;
	total = 0;
	for (int r = 0; r < ROUNDS; r++)
	{
		t1 = rdtscp();
			clock_gettime(CLOCK_MONOTONIC, &start);
		t2 = rdtscp();
		tt = t2-t1;
		measurement[r] = tt;
		total = total + tt;
	}
	output = fopen("clock_gettime.txt", "w");
	for(int r = 0; r < ROUNDS; r++)
		fprintf(output, "%u\n", measurement[r]);
	printf("clock_gettime\t%lu\n", total / ROUNDS);
	fclose(output);
	
	// mincore
	#define PAGE_COUNT 1
	#define PAGE_SIZE 4096
	uint8_t * buffer;
	unsigned char vec[PAGE_COUNT];
	buffer = (uint8_t *)mmap(NULL, PAGE_COUNT * PAGE_SIZE, PROT_READ | PROT_WRITE,  MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	size_t length = sysconf(_SC_PAGESIZE);
	// assert(res == 0);
	for (int r = 0; r < ROUNDS; r++)
	{
		t1 = rdtscp();
			mincore(buffer, length, vec);
		t2 = rdtscp();
		tt = t2-t1;
		measurement[r] = tt;
		total = total + tt;
	}
	output = fopen("mincore.txt", "w");
	for(int r = 0; r < ROUNDS; r++)
		fprintf(output, "%u\n", measurement[r]);
	printf("mincore\t\t%lu\n", total / ROUNDS);
	fclose(output);
	
	return 0;
}
