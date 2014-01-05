#ifdef __cplusplus
extern "C" {
#endif

#include "basic.h"

struct simplefu_semaphore {
    int avail;
    int waiters;
};
typedef struct simplefu_semaphore simplefu;

int futex_lock(void *context);
int futex_unlock(void *context);


struct futex_context {
    struct simplefu_semaphore top_sema;
    struct simplefu_semaphore bottom_sema;
};

void *futex_test_init(int mmap_fd);
void futex_test_destroy(void *context);
    

simplefu *get_top_sema(void *context);
simplefu *get_bottom_sema(void *context);

#ifdef __cplusplus
}
#endif
