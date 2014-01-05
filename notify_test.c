#include <stdio.h>

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "simple_futex.h"

//#define MAX_COUNT 100000
int max_count = 10000;

int dowait = 0;

extern struct lock_ops futex_ops;
extern struct test_ops futex_test_ops;

int main(int argc, char *argv[])
{
    const struct lock_ops *ops = &futex_ops;
    struct test_ops *tops = &futex_test_ops;
	if (argc > 1)
		max_count = atoi(argv[1]);

    if (argc > 2) {
        if (strstr(argv[2], "futex") != NULL) {
            ops = &futex_ops;
            tops = &futex_test_ops;
        }
#if 0
        if (strstr(argv[2], "pthread_cond") != NULL) {
            ops = &pthread_cond_ops;
            tops = &pthread_cond_test_ops;
        }
#endif
    }
        

	int lockfile = open("ipc_lock", O_RDWR);
	assert(lockfile != -1);

    struct futex_context *fctxt;
    fctxt = tops->init(lockfile);
    assert(fctxt != NULL);

    int pcount = 0;
    int ccount = 0;

	int pid = fork();
	assert(pid != -1);

	struct timeval old, new;
	gettimeofday(&new, NULL);

	uint64_t unlock_usecs, lock_usecs;
    unlock_usecs = lock_usecs = 0;

	if( pid == 0 ) { // child
		printf("Initializing...\n");
		//sleep(10);
		printf("Done initializing, max_count:%d\n", max_count);

		while (pcount < max_count) {
			gettimeofday(&old, NULL);
            ops->unlock(tops->get_top_lock((void *)tops));
			gettimeofday(&new, NULL);
			unlock_usecs += (new.tv_sec-old.tv_sec)*1000000+(new.tv_usec-old.tv_usec);
			pcount++;
			//printf("up once\t");

			gettimeofday(&old, NULL);
            ops->lock(tops->get_bottom_lock((void *)tops));
			gettimeofday(&new, NULL);
			lock_usecs += (new.tv_sec-old.tv_sec)*1000000+(new.tv_usec-old.tv_usec);
		}

	    printf("Parent %d done, up avg:%ld, down avg:%ld, dowait:%u\n", getpid(), unlock_usecs/max_count, lock_usecs/max_count, dowait);
		exit(0);
	}

	printf("Waiting for child...\n");


	while (ccount < max_count)
	{
        //usleep(10);
        gettimeofday(&old, NULL);
        ops->lock(tops->get_top_lock((void *)tops));
        gettimeofday(&new, NULL);
        lock_usecs += (new.tv_sec-old.tv_sec)*1000000+(new.tv_usec-old.tv_usec);
		ccount++;

		gettimeofday(&old, NULL);
        ops->unlock(tops->get_bottom_lock((void *)tops));
		gettimeofday(&new, NULL);
		unlock_usecs += (new.tv_sec-old.tv_sec)*1000000+(new.tv_usec-old.tv_usec);
	}
	printf("Child %d done, up avg:%ld, down avg:%ld, dowait:%d\n", getpid(), unlock_usecs/max_count, lock_usecs/max_count, dowait);

	return 0;
}
