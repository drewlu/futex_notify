#include <stdlib.h>
#include <stdio.h>
#include "simple_futex.h"
#include <linux/futex.h>
#include <sys/syscall.h>

extern int dowait;
void simplefu_down(simplefu *who)
{
	int val;
	do {
		val = who->avail;
		if( val > 0 && __sync_bool_compare_and_swap(&who->avail, val, val - 1) )
		{
			//printf("pid:%d val:%d bypass futex, aval:%d\n", getpid(), val, who->avail);
			return;
		}

        //printf("enter FUTEX_WAIT pid:%d\n", getpid());
        __sync_fetch_and_add(&dowait, 1);
		__sync_fetch_and_add(&who->waiters, 1);
		syscall(__NR_futex, &who->avail, FUTEX_WAIT, val, NULL, 0, 0);
		__sync_fetch_and_sub(&who->waiters, 1);
		//printf("val:%d bypass futex, aval:%d\t", val, who->avail);
	} while(1);
}

void simplefu_up(simplefu *who)
{
	int nval = __sync_add_and_fetch(&who->avail, 1);
    //printf("waiters:%d for pid:%d\n", who->waiters, getpid());
	if( who->waiters > 0 )
	{
		int nr = syscall(__NR_futex, &who->avail, FUTEX_WAKE, nval, NULL, 0, 0);
        //printf("FUTEX_WAIT return %d, pid:%d\n", nr, getpid());
	}
}
