#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

#include "threadpool.h"

/*Initializing Pool*/
threadpool* thr_pool_init(int num_threads){

    threadpool* thr_pool = (threadpool*)malloc(sizeof(threadpool));

    pthread_t* threads=(pthread_t*)malloc(num_threads*sizeof(pthread_t));

    (thr_pool)->thr_p = threads;
    (thr_pool)->count = 0;
    (thr_pool)->num_threads = num_threads;
    (thr_pool)->JobQueue = q_init();
    (thr_pool)->finished = 0;

    (thr_pool)->statistics = (stats*)malloc(sizeof(stats));
    (thr_pool)->statistics->bytes=0;
    (thr_pool)->statistics->pages=0;

    pthread_mutex_init(&(thr_pool->lock), NULL);
    pthread_cond_init(&(thr_pool->cond), NULL);

    for(int i=0;i<num_threads;i++){
        pthread_create(&thr_pool->thr_p[i], NULL, thr_job, (void*)thr_pool);
        pthread_detach(thr_pool->thr_p[i]);
    }

    return thr_pool;
}

/*Adding to JobQueue*/
void thr_pool_add(threadpool* pool, int fd, int size, char* file){

    pthread_mutex_lock(&(pool->lock));

    Job *newJob = (Job*)malloc(sizeof(Job));
    newJob->fd = fd;
    newJob->file = strdup(file);
    newJob->parser = newJob->file;
    newJob->file = strtok(newJob->file, "/");
    newJob->file = strtok(NULL," ");
    newJob->next = NULL;

    enqueue(pool->JobQueue,newJob);
    pool->count++;

    pthread_cond_broadcast(&(pool->cond));

    pthread_mutex_unlock(&(pool->lock));
}

/*Waiting for threads to exit*/
void thr_pool_destroy(threadpool* pool, int num_threads){

    pool->finished = 1;
    pthread_cond_broadcast(&(pool->cond));
    for(int i = 0; i < num_threads; i++)
       pthread_join(pool->thr_p[i], NULL);

    thr_pool_free(pool);
}

/*Deallocating memory*/
void thr_pool_free(threadpool* pool){

    free(pool->thr_p);
    q_destroy(pool->JobQueue);
    free(pool->JobQueue);
    free(pool->statistics);

    pthread_mutex_lock(&(pool->lock));
    pthread_mutex_destroy(&(pool->lock));
    pthread_cond_destroy(&(pool->cond));

    free(pool);
}
