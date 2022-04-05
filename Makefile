all:
	gcc -o syscall_benchmark syscall_benchmark.c -lkeyutils -lnuma
clean:
	rm syscall_benchmark
