#ifdef __cplusplus
extern "C" {
#endif

#include "basic.h"

struct simplefu_semaphore {
    int avail;
    int waiters;
};
typedef struct simplefu_semaphore simplefu;

struct futex_context {
    struct simplefu_semaphore top_lock;
    struct simplefu_semaphore bottom_lock;
};


#ifdef __cplusplus
}
#endif
