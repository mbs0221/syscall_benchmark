// $ gcc -o syscall_benchmark syscall_benchmark.c -lkeyutils -lnuma

#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>		// uint64_t
#include <sys/types.h>
#include <time.h>		// time(), clock_gettime()
#include <fcntl.h>		// open
#include <sys/ioctl.h>
#include <sys/socket.h>
#ifdef HAVE_KEYUTILS_H
#include <keyutils.h>	// add_key
#endif
#include <stdlib.h>
#ifdef HAVE_NUMAIF_H
#include <numaif.h>		// get_mempolicy
#endif
#include <sys/mman.h>	// pkey_alloc, pkeyfree
#include <unistd.h>
#include <assert.h>

#define rdtscp()({unsigned int result; asm volatile("rdtscp":"=a"(result)::"rdx","rcx","memory"); result;})

#define PAGE_COUNT 1
#define PAGE_SIZE 4096

#define ROUNDS 1

typedef struct {
	long arg1, arg2, arg3, arg4, arg5, arg6;
} SyscallArgs;

typedef struct {
	long result;
} SyscallResult;

typedef struct {
	SyscallArgs args;
	SyscallResult result;
} BenchmarkState;

typedef struct {
	const char* name;
	void (*setup)(SyscallArgs *args);
	void (*benchmark)(SyscallArgs *args, SyscallResult *result);
	void (*cleanup)(SyscallArgs *args, SyscallResult *result);
} Benchmark;

uint32_t runBenchmarkOnce(const Benchmark* benchmark, BenchmarkState *state) {
	SyscallArgs *args = &state->args;
	SyscallResult *result = &state->result;
	uint32_t t1, t2, tt;

	benchmark->setup(args);
	t1 = rdtscp();
	benchmark->benchmark(args, result);
	t2 = rdtscp();
	benchmark->cleanup(args, result);
	tt = t2 - t1;

	return tt;
}

void runBenchmark(const Benchmark* benchmark, uint32_t* measurement) {

	BenchmarkState state;
	uint32_t t1, t2, tt;
	uint64_t total = 0;

	for (int r = 0; r < ROUNDS; r++) {
		tt = runBenchmarkOnce(benchmark, &state);
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

void setupNothing(SyscallArgs *args) {
	// Nothing here
}

void benchmarkNothing(SyscallArgs *args, SyscallResult *result) {
	// Nothing here
}

void cleanupNothing(SyscallArgs *args, SyscallResult *result) {
	// Nothing here
}

void benchmarkGetPid(SyscallArgs *args, SyscallResult *result) {
	getpid();
}

void benchmarkGetUid(SyscallArgs *args, SyscallResult *result) {
	getuid();
}

void benchmarkTime(SyscallArgs *args, SyscallResult *result) {
	time(NULL);
}

// Read 4096 bytes from /dev/null into buffer
void setupRead(SyscallArgs *args) {
	int fd;
	fd = open("/dev/null", O_RDONLY);
	args->arg1 = fd;
	args->arg2 = malloc(4096);
	args->arg3 = 4096;
}

void benchmarkRead(SyscallArgs *args, SyscallResult *result) {
	int fd = args->arg1;
	result->result = read(fd, args->arg2, args->arg3);
}

void cleanupRead(SyscallArgs *args, SyscallResult *result) {
	// Free buffer
	if (args->arg2) {
		free(args->arg2);
	}

	// Close file descriptor
	assert(result->result != -1);
	if (result->result != -1) {
		close(args->arg1);
	}
}

void setupWrite(SyscallArgs *args) {
	int fd;
	fd = open("/dev/null", O_WRONLY);
	args->arg1 = fd;
	args->arg2 = malloc(4096);
	args->arg3 = 4096;
	sprintf((char*)args->arg2, "Hello, world!");
}

void benchmarkWrite(SyscallArgs *args, SyscallResult *result) {
	int fd = args->arg1;
	char* buf = (char*)args->arg2;
	size_t len = args->arg3;
	result->result = write(fd, buf, len);
}

void cleanupWrite(SyscallArgs *args, SyscallResult *result) {
	// Free buffer
	if (args->arg2) {
		free(args->arg2);
	}

	// Close file descriptor
	assert(result->result != -1);
	if (result->result != -1) {
		close(args->arg1);
	}
}

void benchmarkOpen(SyscallArgs *args, SyscallResult *result) {
	int fd = open("/dev/null", O_RDONLY);
	result->result = fd;
}

void cleanupOpen(SyscallArgs *args, SyscallResult *result) {
	// Close file descriptor
	assert(result->result != -1);
	if (result->result != -1) {
		close(result->result);
	}
}

void setupClose(SyscallArgs *args) {
	int fd;
	fd = open("/dev/null", O_RDONLY);
	args->arg1 = fd;
}

void benchmarkClose(SyscallArgs *args, SyscallResult *result) {
	int fd = args->arg1;
	assert(fd != -1);
	result->result = close(fd);
}

void setupIoctl(SyscallArgs *args) {
	int fd;
	fd = open("/dev/null", O_RDONLY);
	args->arg1 = fd;
}

void benchmarkIoctl(SyscallArgs *args, SyscallResult *result) {
	int fd = args->arg1;
	ioctl(fd, 0, 0);
}

void cleanupIoctl(SyscallArgs *args, SyscallResult *result) {
	// Close file descriptor
	assert(args->arg1 != -1);
	if (args->arg1 != -1) {
		close(args->arg1);
	}
}

void benchmarkSocket(SyscallArgs *args, SyscallResult *result) {
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	result->result = fd;
}

void cleanupSocket(SyscallArgs *args, SyscallResult *result) {
	// Close file descriptor
	assert(result->result != -1);
	if (result->result != -1) {
		close(result->result);
	}
}

void benchmarkMmap(SyscallArgs *args, SyscallResult *result) {
	uint8_t* buffer;
	buffer = (uint8_t*)mmap(NULL, PAGE_COUNT * PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	result->result = (long)buffer;
}

void cleanupMmap(SyscallArgs *args, SyscallResult *result) {
	assert(result->result != -1);
	if (result->result != -1) {
		munmap((void*)result->result, PAGE_COUNT * PAGE_SIZE);
	}
}

void setupMprotect(SyscallArgs *args) {
	uint8_t* buffer;
	buffer = (uint8_t*)mmap(NULL, PAGE_COUNT * PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	args->arg1 = (long)buffer;
}

void benchmarkMprotect(SyscallArgs *args, SyscallResult *result) {
	int ret;
	uint8_t* buffer = (uint8_t*)args->arg1;
	ret = mprotect(buffer, PAGE_COUNT * PAGE_SIZE, PROT_READ);
	result->result = ret;
}

void cleanupMprotect(SyscallArgs *args, SyscallResult *result) {
	if (args->arg1 != -1) {
		munmap((void*)args->arg1, PAGE_COUNT * PAGE_SIZE);
	}
}

void setupUnmap(SyscallArgs *args) {
	uint8_t* buffer;
	buffer = (uint8_t*)mmap(NULL, PAGE_COUNT * PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	args->arg1 = (long)buffer;
}

void benchmarkUnmap(SyscallArgs *args, SyscallResult *result) {
	if (args->arg1 != -1) {
		munmap((void*)args->arg1, PAGE_COUNT * PAGE_SIZE);
	}
}

void benchmarkPkeyAlloc(SyscallArgs *args, SyscallResult *result) {
	int pkey = pkey_alloc(0, PKEY_DISABLE_ACCESS|PKEY_DISABLE_WRITE);
	result->result = pkey;
}

void cleanupPkeyAlloc(SyscallArgs *args, SyscallResult *result) {
	// Free pkey
	if (result->result != -1) {
		pkey_free(result->result);
	}
}

#ifdef HAVE_KEYUTILS_H
void benchmarkAddKey(SyscallArgs *args, SyscallResult *result) {
	add_key(0, 0, 0, 0, 0);
}
#endif

#ifdef HAVE_NUMAIF_H
void benchmarkGetMempolicy(SyscallArgs *args, SyscallResult *result) {
	get_mempolicy(0, 0, 0, 0, 0);
}
#endif

void benchmarkClockGettime(SyscallArgs *args, SyscallResult *result) {
	struct timespec start;
	clock_gettime(CLOCK_MONOTONIC, &start);
}

void setupMincore(SyscallArgs *args) {
	#define PAGE_COUNT 1
	#define PAGE_SIZE 4096
	uint8_t* buffer;
	unsigned char *vec = (unsigned char*)malloc(PAGE_COUNT);
	buffer = (uint8_t*)mmap(NULL, PAGE_COUNT * PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	size_t length = sysconf(_SC_PAGESIZE);
	args->arg1 = (long)buffer;
	args->arg2 = length;
	args->arg3 = (long)vec;
	// printf("buffer: %p, length: %ld, vec: %p\n", buffer, length, vec);
}

void benchmarkMincore(SyscallArgs *args, SyscallResult *result) {
	uint8_t* buffer = (uint8_t*)args->arg1;
	size_t length = args->arg2;
	unsigned char *vec = (unsigned char*)args->arg3;

	mincore(buffer, length, vec);
}

void cleanupMincore(SyscallArgs *args, SyscallResult *result) {
	if (args->arg1 != -1) {
		munmap((void*)args->arg1, PAGE_COUNT * PAGE_SIZE);
	}
	if (args->arg3 != -1) {
		free((void*)args->arg3);
	}
}

#define BENCHMARK(name, __setup, __benchmark, __cleanup) \
		{#name".txt", setup##__setup, benchmark##__benchmark, cleanup##__cleanup}

int main(int argc, char *argv[]) {
	uint32_t measurement[ROUNDS] = {0};

	// Warmup loop
	for (int i = 0; i < 55555555; i++);

	Benchmark benchmarks[] = {
		BENCHMARK(Nothing, Nothing, Nothing, Nothing),
		BENCHMARK(GetPid, Nothing, GetPid, Nothing),
		BENCHMARK(GetUid, Nothing, GetUid, Nothing),
		BENCHMARK(Read, Read, Read, Read),
		BENCHMARK(Write, Write, Write, Write),
		BENCHMARK(Open, Nothing, Open, Open),
		BENCHMARK(Close, Close, Close, Nothing),
		BENCHMARK(Ioctl, Ioctl, Ioctl, Ioctl),
		BENCHMARK(Socket, Nothing, Socket, Socket),
		BENCHMARK(Mmap, Nothing, Mmap, Mmap),
		BENCHMARK(Mprotect, Mprotect, Mprotect, Mprotect),
		BENCHMARK(Unmap, Unmap, Unmap, Nothing),
		BENCHMARK(PkeyAlloc, Nothing, PkeyAlloc, PkeyAlloc),
#ifdef HAVE_KEYUTILS_H
		BENCHMARK(AddKey, Nothing, AddKey, Nothing),
#endif
#ifdef HAVE_NUMAIF_H
		BENCHMARK(GetMempolicy, Nothing, GetMempolicy, Nothing),
#endif
		BENCHMARK(ClockGettime, Nothing, ClockGettime, Nothing),
		BENCHMARK(Mincore, Mincore, Mincore, Mincore),
		BENCHMARK(Time, Nothing, Time, Nothing),
	};

	int numBenchmarks = sizeof(benchmarks) / sizeof(benchmarks[0]);

	if (argc < 2) {
		printf("Usage: %s <benchmark>\n", argv[0]);
		printf("Available benchmarks:\n");
		for (int i = 0; i < numBenchmarks; i++) {
			printf("%d: %s\n", i, benchmarks[i].name);
		}
		return 1;
	}

	int i = atoi(argv[1]);
	if (i < 0 || i > numBenchmarks) {
		printf("Invalid benchmark index\n");
		return 1;
	}

	if (i == numBenchmarks) {
		for (int j = 0; j < numBenchmarks; j++) {
			runBenchmark(&benchmarks[j], measurement);
		}
	} else {
		runBenchmark(&benchmarks[i], measurement);
	}

	return 0;
}
