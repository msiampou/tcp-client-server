
#include "Queue.h"

typedef struct thread{
    pthread_t pthread;
}thread;

typedef struct statistics{
    int bytes;
    int pages;
}stats;

typedef struct threadpool{
    pthread_t* thr_p;
    int count;
    int num_threads;
    int finished;
    char* directory;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    JQueue* JobQueue;
    stats* statistics;
}threadpool;

void* thr_job(void*);
void thr_work(threadpool*, int);

threadpool* thr_pool_init(int);
void thr_pool_add(threadpool*, int, int, char*);
void thr_pool_destroy(threadpool*, int);
void thr_pool_free(threadpool*);
