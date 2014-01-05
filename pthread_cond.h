#ifdef __cplusplus
extern "C" {
#endif

#include "basic.h"

struct cond_lock {
    pthread_mutex_t lock;
    pthread_cond_t cond;
};
typedef struct simplefu_semaphore simplefu;

struct cond_context {
    struct cond_lock top_lock;
    struct cond_lock bottom_lock;
};

#ifdef __cplusplus
}
#endif
