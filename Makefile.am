bin_PROGRAMS = syscall_benchmark
syscall_benchmark_SOURCES = syscall_benchmark.c
syscall_benchmark_LDADD = -ldl -lrt -lpthread -ldune

if HAVE_KEYUTILS
syscall_benchmark_LDADD += -lkeyutils
endif

if HAVE_NUMA
syscall_benchmark_LDADD += -lnuma
endif