#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "functions.h"

void* thr_job(void* p){

    threadpool* pool = (threadpool*)p;
    int i=0;
    while(pool->finished == 0){

        pthread_mutex_lock(&(pool->lock));
      
        while (pool->JobQueue->count<=0){

            pool->JobQueue->done++;

            if((pool->JobQueue->done>=pool->num_threads) && (i!=0)){
                pool->finished = 1;
                break;
            }
            i++;

            if(pthread_cond_wait(&(pool->cond), &(pool->lock)) !=0)
                printf("error\n");
           
            if(pool->finished == 1)
                break;
            
        }
        if(pool->finished == 1){
            pthread_mutex_unlock(&(pool->lock));
            break;
        }

        printf("Thread with id = %ld ready to check a file from queue\n", pthread_self());
        pool->JobQueue->done = 0;
        Job* task = dequeue(pool->JobQueue);
        printf("-->%s\n",task->url);
        pool->count--;
        pool->socket->sockfd = SocketInit(pool->socket);
        if(task!=NULL){
            char* resp = SocketConnection(pool->socket,task->url,&(pool->statistics->bytes));
            FileCreate(pool, task->url, resp);
            pthread_mutex_unlock(&(pool->lock));
            char* parser = task->url;
            FileManagment(pool,resp, task->url);
            free(parser);
            free(task);
            close(pool->socket->sockfd);
        }
        else
            pthread_mutex_unlock(&(pool->lock));
        
        pthread_cond_broadcast(&(pool->cond));
    }
    printf("Crawling Process Ended\n");
    pthread_exit(NULL);
    return NULL;
}
