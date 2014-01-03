all:
	gcc -O3 -g -c simple_futex.c -o simple_futex.o
	gcc -O3 -g -c futex_test.c -o futex_test.o
	gcc *.o -o futex_test
	dd if=/dev/zero of=ipc_lock bs=8 count=2

clean:
	rm -f *.o futex_test

.PHONY: clean
