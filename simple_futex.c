#include <stdlib.h>
#include <stdio.h>
#include "simple_futex.h"
#include <linux/futex.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <assert.h>

extern int dowait;
extern struct test_ops futex_test_ops;

void *futex_test_init(int mmap_fd)
{
    struct futex_context *context;
    context = mmap(NULL, 2*sizeof(simplefu),
            PROT_READ|PROT_WRITE,
            MAP_SHARED, mmap_fd, 0);
    
    assert(context != MAP_FAILED);

    context->top_sema.avail = 0;
    context->top_sema.waiters = 0;
    context->bottom_sema.avail = 0;
    context->bottom_sema.waiters = 0;

    futex_test_ops.context = context;

    return context;
}

void futex_test_destroy(void *context)
{
    munmap(context, 2*sizeof(simplefu));
}

simplefu *get_top_sema(void *context)
{
    struct test_ops *tops = (struct test_ops *)context;
    struct futex_context *test_cxt = (struct futex_context *)(tops->context);
    return &test_cxt->top_sema;
}

simplefu *get_bottom_sema(void *context)
{
    struct test_ops *tops = (struct test_ops *)context;
    struct futex_context *test_cxt = (struct futex_context *)(tops->context);
    return &test_cxt->bottom_sema;
}


int futex_lock(void *context)
{
    simplefu *who = (simplefu *)context;
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

    return 0;
}

int futex_unlock(void *context)
{
    simplefu *who = (simplefu *)context;
	int nval = __sync_add_and_fetch(&who->avail, 1);
    //printf("waiters:%d for pid:%d\n", who->waiters, getpid());
	if( who->waiters > 0 )
	{
		int nr = syscall(__NR_futex, &who->avail, FUTEX_WAKE, nval, NULL, 0, 0);
        //printf("FUTEX_WAIT return %d, pid:%d\n", nr, getpid());
	}

    return 0;
}

const struct lock_ops futex_ops = {
    .lock = futex_lock,
    .unlock = futex_unlock
};

struct test_ops futex_test_ops = {
    .context = NULL,
    .init = futex_test_init,
    .destroy = futex_test_destroy
};

