all:
	gcc -O3 -g -c simple_futex.c -o simple_futex.o
	gcc -O3 -g -c notify_test.c -o notify_test.o
	gcc *.o -o notify_test
	dd if=/dev/zero of=ipc_lock bs=8 count=2

clean:
	rm -f *.o notify_test

.PHONY: clean
