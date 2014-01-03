#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "simple_futex.h"
//#define MAX_COUNT 100000
int max_count = 10000;

int dowait = 0;

int main(int argc, char *argv[])
{
	if (argc > 1)
		max_count = atoi(argv[1]);

	int lockfile = open("ipc_lock", O_RDWR);
	assert(lockfile != -1);
	simplefu *sema1 = mmap(NULL, 2*sizeof(simplefu), PROT_READ|PROT_WRITE,
			MAP_SHARED, lockfile, 0);
	sema1->avail = 0;
	sema1->waiters = 0;

	simplefu *sema2 = (simplefu *)(sema1++);
	sema2->avail = 0;
	sema2->waiters = 0;

	assert(sema1 != MAP_FAILED);
	int pcount = 0;
	int ccount = 0;

	int pid = fork();
	assert(pid != -1);

	struct timeval old, new;
	gettimeofday(&new, NULL);

	uint64_t up_usecs, down_usecs;
    up_usecs = down_usecs = 0;

	if( pid == 0 ) { // child
		printf("Initializing...\n");
		//sleep(10);
		printf("Done initializing, max_count:%d\n", max_count);
		while (pcount < max_count) {
			gettimeofday(&old, NULL);
			simplefu_up(sema1);
			gettimeofday(&new, NULL);
			up_usecs += (new.tv_sec-old.tv_sec)*1000000+(new.tv_usec-old.tv_usec);
			pcount++;
			//printf("up once\t");

			gettimeofday(&old, NULL);
			simplefu_down(sema2);
			gettimeofday(&new, NULL);
			down_usecs += (new.tv_sec-old.tv_sec)*1000000+(new.tv_usec-old.tv_usec);
		}

	    printf("Parent %d done, up avg:%ld, down avg:%ld, dowait:%u\n", getpid(), up_usecs/max_count, down_usecs/max_count);
		exit(0);
	}
	printf("Waiting for child...\n");
	while (ccount < max_count)
	{
        //usleep(10);
        gettimeofday(&old, NULL);
		simplefu_down(sema1);
        gettimeofday(&new, NULL);
        down_usecs += (new.tv_sec-old.tv_sec)*1000000+(new.tv_usec-old.tv_usec);
		ccount++;

		gettimeofday(&old, NULL);
		simplefu_up(sema2);
		gettimeofday(&new, NULL);
		up_usecs += (new.tv_sec-old.tv_sec)*1000000+(new.tv_usec-old.tv_usec);
	}
	printf("Child %d done, up avg:%ld, down avg:%ld, dowait:%d\n", getpid(), up_usecs/max_count, down_usecs/max_count);

	return 0;
}
