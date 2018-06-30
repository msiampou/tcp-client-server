#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

#include "threadpool.h"

threadpool* thr_pool_init(int num_threads){

    threadpool* thr_pool = (threadpool*)malloc(sizeof(threadpool));

    pthread_t* threads=(pthread_t*)malloc(num_threads*sizeof(pthread_t));
    (thr_pool)->socket = (sock*)malloc(sizeof(sock));

    (thr_pool)->thr_p = threads;
    (thr_pool)->count = 0;
    (thr_pool)->num_threads = num_threads;
    (thr_pool)->JobQueue = q_init();
    (thr_pool)->dummyQueue = q_init();
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

void thr_pool_add(threadpool* pool, char* url){

    pthread_mutex_lock(&(pool->lock));
    printf("\nmutex locked in add\n");

    Job *newJob = (Job*)malloc(sizeof(Job));
    newJob->url = strdup(url);
    newJob->next = NULL;

    enqueue(pool->JobQueue,newJob);
    pool->count++;

    printf("signal\n");
    pthread_cond_signal(&(pool->cond));

    pthread_mutex_unlock(&(pool->lock));
    printf("\nmutex unlocked in add\n");

}

void thr_pool_destroy(threadpool* pool, int num_threads){
    pool->finished = 1;
    pthread_cond_broadcast(&(pool->cond));
    for(int i = 0; i < num_threads; i++)
       pthread_join(pool->thr_p[i], NULL);
    thr_pool_free(pool);
}

void thr_pool_free(threadpool* pool){

    free(pool->thr_p);
    //q_destroy(pool->JobQueue);
    q_destroy(pool->dummyQueue);
    free(pool->JobQueue);
    free(pool->dummyQueue);
    free(pool->statistics);
    free(pool->socket);

    pthread_mutex_lock(&(pool->lock));
    pthread_mutex_destroy(&(pool->lock));
    pthread_cond_destroy(&(pool->cond));

    fclose(pool->fp);

    free(pool);
}
