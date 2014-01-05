#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <assert.h>

#include "pthread_cond.h"

extern int dowait;
extern struct test_ops futex_test_ops;

void *pcond_test_init(int mmap_fd)
{
    struct pcond_context *context;
    context = mmap(NULL, 2*sizeof(pcond_lock),
            PROT_READ|PROT_WRITE,
            MAP_SHARED, mmap_fd, 0);
    
    assert(context != MAP_FAILED);

    context->top_lock.lock = PTHREAD_MUTEX_INITIALIZER;
    context->bottom_lock.lock = PTHREAD_MUTEX_INITIALIZER;
    context->top_lock.cond = PTHREAD_COND_INITIALIZER;
    context->bottom_lock.cond = PTHREAD_COND_INITIALIZER;

    pcond_test_ops.context = context;

    return context;
}

void pcond_test_destroy(void *context)
{
    munmap(context, sizeof(struct pcond_context));
}

void *pcond_get_top_lock(void *context)
{
    struct test_ops *tops = (struct test_ops *)context;
    struct pcond_context *test_cxt = (struct pcond_context *)(tops->context);
    return &test_cxt->top_lock;
}

void *pcond_get_bottom_lock(void *context)
{
    struct test_ops *tops = (struct test_ops *)context;
    struct futex_context *test_cxt = (struct futex_context *)(tops->context);
    return &test_cxt->bottom_lock;
}


int pcond_lock(void *context)
{
    pcond_lock *who = (pcond_lock *)context;

    pthread_mutex_lock(&who->lock);
    pthread_cond_wait(&who->cond, &who->lock);
    pthread_mutex_unlock(&who->lock);

    return 0;
}

int pcond_unlock(void *context)
{
    pcond_lock *who = (pcond_lock *)context;

    pthread_mutex_lock(&who->lock);
    pthread_cond_signal(&who->cond, &who->lock);
    pthread_mutex_unlock(&who->lock);
    
    return 0;
}

const struct lock_ops pcond_ops = {
    .lock = pcond_lock,
    .unlock = pcond_unlock
};

struct test_ops pcond_test_ops = {
    .context = NULL,
    .init = pcond_test_init,
    .destroy = pcond_test_destroy,
    .get_top_lock = pcond_get_top_lock,
    .get_bottom_lock = pcond_get_bottom_lock
};
