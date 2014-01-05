#ifdef __cplusplus
extern "C" {
#endif

struct lock_ops
{
    int (*lock)(void *context);
    int (*unlock)(void *context);
};

struct test_ops
{
    void *context;
    void *(*init)(int mmap_fd);
    void (*destroy)(void *context);
    void *(*get_top_lock)(void *context);
    void *(*get_bottom_lock)(void *context);
};

#ifdef __cplusplus
}
#endif
