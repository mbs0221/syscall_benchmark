// $ gcc -o syscall_benchmark syscall_benchmark.c -lkeyutils -lnuma

#include <stdio.h>
#include <stdint.h>		// uint64_t
#include <sys/types.h>
#include <time.h>		// time(), clock_gettime()
#include <sys/ioctl.h>
#ifdef HAVE_KEYUTILS_H
#include <keyutils.h>	// add_key
#endif
#include <stdlib.h>
#ifdef HAVE_NUMAIF_H
#include <numaif.h>		// get_mempolicy
#endif
#include <sys/mman.h>	// pkey_alloc, pkeyfree
#include <unistd.h>

#define rdtscp()({unsigned int result; asm volatile("rdtscp":"=a"(result)::"rdx","rcx","memory"); result;})

#define ROUNDS 1000000

typedef struct {
	const char* name;
	void (*benchmark)(uint32_t*);
} Benchmark;

void runBenchmark(const Benchmark* benchmark, uint32_t* measurement) {
	uint32_t t1, t2, tt;
	uint64_t total = 0;

	for (int r = 0; r < ROUNDS; r++) {
		t1 = rdtscp();
		benchmark->benchmark(measurement);
		t2 = rdtscp();
		tt = t2 - t1;
		measurement[r] = tt;
		total += tt;
	}

	FILE* output = fopen(benchmark->name, "w");
	for (int r = 0; r < ROUNDS; r++) {
		fprintf(output, "%u\n", measurement[r]);
	}
	printf("%s\t\t%lu\n", benchmark->name, total / ROUNDS);
	fclose(output);
}

void benchmarkNothing(uint32_t* measurement) {
	// Nothing here
}

void benchmarkGetPid(uint32_t* measurement) {
	getpid();
}

void benchmarkGetUid(uint32_t* measurement) {
	getuid();
}

void benchmarkTime(uint32_t* measurement) {
	time(NULL);
}

void benchmarkRead(uint32_t* measurement) {
	int fd;
	read(fd, 0, 0);
}

void benchmarkIoctl(uint32_t* measurement) {
	int fd;
	ioctl(fd, 0, 0);
}

#ifdef HAVE_KEYUTILS_H
void benchmarkAddKey(uint32_t* measurement) {
	add_key(0, 0, 0, 0, 0);
}
#endif

#ifdef HAVE_NUMAIF_H
void benchmarkGetMempolicy(uint32_t* measurement) {
	get_mempolicy(0, 0, 0, 0, 0);
}
#endif

void benchmarkClockGettime(uint32_t* measurement) {
	struct timespec start;
	clock_gettime(CLOCK_MONOTONIC, &start);
}

void benchmarkMincore(uint32_t* measurement) {
	#define PAGE_COUNT 1
	#define PAGE_SIZE 4096
	uint8_t* buffer;
	unsigned char vec[PAGE_COUNT];
	buffer = (uint8_t*)mmap(NULL, PAGE_COUNT * PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	size_t length = sysconf(_SC_PAGESIZE);

	mincore(buffer, length, vec);
}

int main(void) {
	uint32_t measurement[ROUNDS] = {0};

	// Warmup loop
	for (int i = 0; i < 55555555; i++);

	Benchmark benchmarks[] = {
		{"nothing.txt", benchmarkNothing},
		{"getpid.txt", benchmarkGetPid},
		{"getuid.txt", benchmarkGetUid},
		{"time.txt", benchmarkTime},
		{"read.txt", benchmarkRead},
		{"ioctl.txt", benchmarkIoctl},
#ifdef HAVE_KEYUTILS_H
		{"add_key.txt", benchmarkAddKey},
#endif
#ifdef HAVE_NUMAIF_H
		{"get_mempolicy.txt", benchmarkGetMempolicy},
#endif
		{"clock_gettime.txt", benchmarkClockGettime},
		{"mincore.txt", benchmarkMincore}
	};

	int numBenchmarks = sizeof(benchmarks) / sizeof(benchmarks[0]);

	for (int i = 0; i < numBenchmarks; i++) {
		runBenchmark(&benchmarks[i], measurement);
	}

	return 0;
}
